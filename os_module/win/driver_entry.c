// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
//
// SecAgent - KMDF (WDF) Non-PnP control device, IOCTL-only

#include <ntddk.h>
#include <wdf.h>

#include "driver_entry.h"
#include "win_intr.h"
#include <sa_headers.h>

// // WPP (optional): keep if you already have it wired
#include "driver_entry.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif

typedef struct _SA_REQ_CTX
{
    WDFMEMORY u_mem; // holds MDL + mapping lifetime
    PVOID u_mem_kva;     // system-space mapping to user pages
    SIZE_T u_mem_len;    // bytes locked/mapped
    PVOID u_mem_uva;     // original user VA (for debugging)
} SA_REQ_CTX, *PSA_REQ_CTX;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SA_REQ_CTX, SaGetReqCtx);

// -----------------------
// Globals
// -----------------------
static WDFDEVICE g_WdfDevice = NULL;

static EVT_WDF_IO_IN_CALLER_CONTEXT SaEvtIoInCallerContext;

static OSSTATUS sa_win_u_map_mem(WDFREQUEST Request, PSA_REQ_CTX ctx, PVOID u_mem_uva, SIZE_T u_mem_len, NTSTATUS* out_nt_status)
{
    NTSTATUS nt_status;

    // Allocate request context
    WDF_OBJECT_ATTRIBUTES attrs;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attrs, SA_REQ_CTX);

    nt_status = WdfObjectAllocateContext(Request, &attrs, (PVOID *)&ctx);
    if (!NT_SUCCESS(nt_status))
    {
        *out_nt_status = nt_status;
        return E_INTERNAL_ERROR;
    }

    ctx->u_mem_uva = u_mem_uva;
    ctx->u_mem_len = u_mem_len;

    // Give read + write access permission to the user memory.
    //// Requestor must be user-mode; if kernel-mode caller, treat differently.
    //// (If you have kernel callers, check WdfRequestGetRequestorMode(Request).)
    nt_status = WdfRequestProbeAndLockUserBufferForWrite(
        Request,
        u_mem_uva,
        u_mem_len,
        &ctx->u_mem);
    if (!NT_SUCCESS(nt_status))
    {
        *out_nt_status = nt_status;
        return E_INTERNAL_ERROR;
    }

    ctx->u_mem_kva = WdfMemoryGetBuffer(ctx->u_mem, NULL);
    if (!ctx->u_mem_kva)
    {
        *out_nt_status = STATUS_INSUFFICIENT_RESOURCES;
        return E_INTERNAL_ERROR;
    }

    // On success
    *out_nt_status = nt_status;
    return OS_SUCCESS;
}

static VOID SaEvtIoInCallerContext(WDFDEVICE Device, WDFREQUEST Request)
{
    OSSTATUS status = OS_SUCCESS;
    NTSTATUS nt_status;
    WDF_REQUEST_PARAMETERS params;
    struct tz_cmd_packet *cmd = NULL;
    size_t cmdLen = 0;

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    // Only preprocess your IOCTL
    if (params.Type != WdfRequestTypeDeviceControl ||
        params.Parameters.DeviceIoControl.IoControlCode != IOCTL_SECAGENT)
    {
        nt_status = WdfDeviceEnqueueRequest(Device, Request);
        if (!NT_SUCCESS(nt_status))
            WdfRequestComplete(Request, nt_status);
        return;
    }

    // METHOD_BUFFERED: input lives in SystemBuffer; safe to read here
    nt_status = WdfRequestRetrieveInputBuffer(Request, sizeof(*cmd), (PVOID *)&cmd, &cmdLen);
    if (!NT_SUCCESS(nt_status))
    {
        WdfRequestComplete(Request, nt_status);
        return;
    }

    // If a SecAgent command takes input/output user memory, map the memory in kernel as required by WDF.
    //// Allocate request context
    WDF_OBJECT_ATTRIBUTES attrs;
    PSA_REQ_CTX ctx = NULL;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attrs, SA_REQ_CTX);

    nt_status = WdfObjectAllocateContext(Request, &attrs, (PVOID *)&ctx);
    if (!NT_SUCCESS(nt_status))
    {
        WdfRequestComplete(Request, nt_status);
        return;
    }

    switch (cmd->id)
    {
        case CMD_POLL_OS_NOTIFICATION_INFO:
            status = sa_win_u_map_mem(Request, ctx, (PVOID)(ULONG_PTR)cmd->arg1, sizeof(struct sa_app_notif_info), &nt_status);
            break;
        case CMD_REGISTER_OS_NOTIFICATION:
            status = sa_win_u_map_mem(Request, ctx, (PVOID)(ULONG_PTR)cmd->arg1, sizeof(struct sa_app_notif_reginfo), &nt_status);
            break;
        case CMD_REGISTER_SECOS:
            status = sa_win_u_map_mem(Request, ctx, (PVOID)(ULONG_PTR)cmd->arg1, sizeof(struct sl_sos_sect_addrs), &nt_status);
            break;
        case CMD_REGISTER_SECPROCESS_MEMORY:
            status = sa_win_u_map_mem(Request, ctx, (PVOID)(ULONG_PTR)cmd->arg1, sizeof(struct hv_reg_sections), &nt_status);
            break;
        case CMD_REGISTER_SECPROCESS:
            status = sa_win_u_map_mem(Request, ctx, (PVOID)(ULONG_PTR)cmd->arg1, sizeof(SP_USTRUCT), &nt_status);
            break;      
        case CMD_SECPROCESS_GET_IDS_AND_NAMES:
            status = sa_win_u_map_mem(Request, ctx, (PVOID)(ULONG_PTR)cmd->arg1, sizeof(struct sp_info_id_name_resp), &nt_status);
            break;
        default:
            // No user memory needs to be mapped, set <status> to be SUCCESS to pass the check of <status> 
            status = OS_SUCCESS;
            break;
    }

    if (SA_FAIL(status))
    {
        DRV_ERR("[Windows] Failed to map user memory! cmd: %u\n", cmd->id);
        WdfRequestComplete(Request, nt_status);
        return;
    }

    // Hand off to normal queue processing
    nt_status = WdfDeviceEnqueueRequest(Device, Request);
    if (!NT_SUCCESS(nt_status))
        WdfRequestComplete(Request, nt_status);
}

// -----------------------
// Core IOCTL logic (no completion here)
// METHOD_BUFFERED => use the same in/out buffer
// -----------------------
static NTSTATUS
SaProcessIoctlBuffered(
    IN ULONG IoControlCode,
    _Inout_updates_bytes_(BufferLen) PVOID Buffer,
    IN size_t InLen,
    IN size_t OutLen,
    OUT size_t *BytesWritten,
    IN PSA_REQ_CTX Ctx
)
{
    OSSTATUS st = OS_SUCCESS;
    struct tz_cmd_packet *cmd_packet = NULL;
    struct tz_cmd_packet *in_cmd_packet = (struct tz_cmd_packet *)Buffer;

    *BytesWritten = 0;

    if (IoControlCode != IOCTL_SECAGENT)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    // Check: SecAgent must be running.
    if (!secagent_is_running())
    {
        return STATUS_UNSUCCESSFUL;
    }

    if (Buffer == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (InLen < sizeof(struct tz_cmd_packet) || OutLen < sizeof(struct tz_cmd_packet))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Copy the input buffer to prevent TOCTTOU attacks
    // WDF framework may copy the input buffer, but SecAgent does not trust it to reduce reliance on it.
    cmd_packet = (struct tz_cmd_packet *)os->mm_malloc_zeroed(sizeof(struct tz_cmd_packet));
    if (!cmd_packet)
    {
        DRV_ERR("<cmd_packet> allocation error!\n");
        in_cmd_packet->status = E_NO_MEM;
        goto finish;
    }

    os->mm_memcpy(cmd_packet, in_cmd_packet, sizeof(struct tz_cmd_packet));

    // WDF may run SecAgent in a different thread context from the caller applicatin. Thus, we need to replace user 
    // memory pointers in <cmd_packet> with the kernel va that maps the memory before accessing the memory.
    switch (cmd_packet->id)
    {
        case CMD_POLL_OS_NOTIFICATION_INFO:
        case CMD_REGISTER_OS_NOTIFICATION:
        case CMD_REGISTER_SECOS:
        case CMD_REGISTER_SECPROCESS_MEMORY:
        case CMD_REGISTER_SECPROCESS:
        case CMD_SECPROCESS_GET_IDS_AND_NAMES:
            if(Ctx)
            {
                cmd_packet->arg1 = (uint64_t)Ctx->u_mem_kva;
            }
            break;
        default:
            // No address needs to be replaced
            break;
    }

    // Call the main ioctl handler
    int copy_retval = 1;
    st = ioctl_request_handler(cmd_packet, &copy_retval);
    cmd_packet->status = st;

    // Copy back the result
    os->mm_memcpy(Buffer, cmd_packet, sizeof(struct tz_cmd_packet));

finish:
    // Transport succeeded; command result is in (struct tz_cmd_packet*)Buffer->status
    *BytesWritten = sizeof(struct tz_cmd_packet);
    return STATUS_SUCCESS;
}

// -----------------------
// WDF EvtIoDeviceControl => calls SaProcessIoctlBuffered
// -----------------------
static VOID
SaEvtIoDeviceControl(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t OutputBufferLength,
    IN size_t InputBufferLength,
    IN ULONG IoControlCode)
{
    UNREFERENCED_PARAMETER(Queue);
    PAGED_CODE();

    NTSTATUS nt;
    PVOID outBuf = NULL;
    size_t outBufLen = 0;
    PVOID inBuf = NULL;
    size_t inBufLen = 0;

    // Check: The function must be called at PASSIVE_LEVEL.
    if (PASSIVE_LEVEL != KeGetCurrentIrql())
    {
        DRV_ERR("IOCTL must be called at PASSIVE_LEVEL!\n");
        return;
    }

    // METHOD_BUFFERED: both retrieve calls usually return the same SystemBuffer pointer.
    nt = WdfRequestRetrieveInputBuffer(Request, 0, &inBuf, &inBufLen);
    if (!NT_SUCCESS(nt))
    {
        WdfRequestComplete(Request, nt);
        return;
    }

    nt = WdfRequestRetrieveOutputBuffer(Request, 0, &outBuf, &outBufLen);
    if (!NT_SUCCESS(nt))
    {
        WdfRequestComplete(Request, nt);
        return;
    }

    // Use the framework-provided lengths (InputBufferLength/OutputBufferLength),
    // but also clamp to retrieved buffer sizes to be safe.
    size_t inLen = (size_t)min((ULONG_PTR)InputBufferLength, (ULONG_PTR)inBufLen);
    size_t outLen = (size_t)min((ULONG_PTR)OutputBufferLength, (ULONG_PTR)outBufLen);

    PSA_REQ_CTX ctx = NULL;
    __try {
        ctx = SaGetReqCtx(Request);  // will be NULL if not allocated
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ctx = NULL;
    }

    // For METHOD_BUFFERED we want the single in/out system buffer.
    // Prefer outBuf (same as inBuf) as the inout buffer.
    size_t bytes = 0;
    nt = SaProcessIoctlBuffered(IoControlCode, outBuf, inLen, outLen, &bytes, ctx);

    WdfRequestCompleteWithInformation(Request, nt, bytes);
}

// -----------------------
// Unload / Cleanup
// -----------------------
static VOID
SaEvtDriverUnload(IN WDFDRIVER Driver)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(Driver);

    // Your cleanup order (same as your WDM version)
    secagent_fini();
    win_intr_fini();

    // For a control device, deleting the device is OK; framework removes symlink too.
    if (g_WdfDevice != NULL)
    {
        WdfObjectDelete(g_WdfDevice);
        g_WdfDevice = NULL;
    }
}

static VOID
SaEvtDriverContextCleanup(IN WDFOBJECT DriverObject)
{
    PAGED_CODE();

    // WPP cleanup (optional)
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)DriverObject));
}

// -----------------------
// Create the control device (Non-PnP)
// -----------------------
static NTSTATUS
SaCreateControlDevice(IN WDFDRIVER Driver)
{
    PAGED_CODE();

    NTSTATUS nt;
    PWDFDEVICE_INIT pInit = NULL;
    WDF_IO_QUEUE_CONFIG qCfg;
    WDF_OBJECT_ATTRIBUTES devAttrs;
    WDF_OBJECT_ATTRIBUTES qAttrs;
    WDFQUEUE queue;

    DECLARE_CONST_UNICODE_STRING(ntDeviceName, SA_NTDEVICE_NAME_STRING);
    DECLARE_CONST_UNICODE_STRING(symLinkName, SA_SYMBOLIC_NAME_STRING);

    // Allocate init for a control device
    pInit = WdfControlDeviceInitAllocate(
        Driver,
        &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
    if (pInit == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Register IoInCallerContext callback
    WdfDeviceInitSetIoInCallerContextCallback(pInit, SaEvtIoInCallerContext);

    // Enforce buffered I/O because your IOCTL path expects a SystemBuffer-like buffer.
    WdfDeviceInitSetIoType(pInit, WdfDeviceIoBuffered);

    // Optional: exclusive open
    // WdfDeviceInitSetExclusive(pInit, TRUE);

    nt = WdfDeviceInitAssignName(pInit, &ntDeviceName);
    if (!NT_SUCCESS(nt))
    {
        WdfDeviceInitFree(pInit);
        return nt;
    }

    // No FileCreate/FileClose: we do NOT set file object config.
    // WDF will default-complete create/close/cleanup successfully.

    // Create device
    WDF_OBJECT_ATTRIBUTES_INIT(&devAttrs);

    nt = WdfDeviceCreate(&pInit, &devAttrs, &g_WdfDevice);
    if (!NT_SUCCESS(nt))
    {
        if (pInit != NULL)
            WdfDeviceInitFree(pInit);
        return nt;
    }

    // Create symbolic link for user-mode opens
    nt = WdfDeviceCreateSymbolicLink(g_WdfDevice, &symLinkName);
    if (!NT_SUCCESS(nt))
    {
        return nt;
    }

    // Create default queue (IOCTL-only)
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&qCfg, WdfIoQueueDispatchSequential);
    qCfg.EvtIoDeviceControl = SaEvtIoDeviceControl;

    // Control devices are typically not power-managed
    qCfg.PowerManaged = WdfFalse;

    WDF_OBJECT_ATTRIBUTES_INIT(&qAttrs);

    // Ensure IOCTL callback runs at PASSIVE_LEVEL (matches your original assumptions)
    qAttrs.ExecutionLevel = WdfExecutionLevelPassive;

    nt = WdfIoQueueCreate(g_WdfDevice, &qCfg, &qAttrs, &queue);
    if (!NT_SUCCESS(nt))
    {
        return nt;
    }

    // Finish initialization so I/O is accepted
    WdfControlFinishInitializing(g_WdfDevice);
    return STATUS_SUCCESS;
}

// -----------------------
// DriverEntry
// -----------------------
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    PAGED_CODE();

    NTSTATUS nt;
    WDF_DRIVER_CONFIG cfg;
    WDF_OBJECT_ATTRIBUTES drvAttrs;
    WDFDRIVER wdfDriver;

    // Configure as Non-PnP KMDF driver
    WDF_DRIVER_CONFIG_INIT(&cfg, WDF_NO_EVENT_CALLBACK);
    cfg.DriverInitFlags |= WdfDriverInitNonPnpDriver;
    cfg.EvtDriverUnload = SaEvtDriverUnload;

    WDF_OBJECT_ATTRIBUTES_INIT(&drvAttrs);
    drvAttrs.EvtCleanupCallback = SaEvtDriverContextCleanup;

    nt = WdfDriverCreate(DriverObject, RegistryPath, &drvAttrs, &cfg, &wdfDriver);
    if (!NT_SUCCESS(nt))
    {
        secagent_mark_not_running();
        // Keep your legacy behavior if you insist: load succeeds even if init fails
        return STATUS_SUCCESS;
    }

    // Start WPP after WdfDriverCreate (matches sample guidance)
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    // 1) Create WDF control device
    nt = SaCreateControlDevice(wdfDriver);
    if (!NT_SUCCESS(nt))
    {
        secagent_mark_not_running();
        return STATUS_SUCCESS; // legacy behavior
    }

    // 2) Initialize interrupts + SecAgent core AFTER WDF device exists
    //    (this matches your requirement ordering)
    {
        PDEVICE_OBJECT wdmDevObj = WdfDeviceWdmGetDeviceObject(g_WdfDevice);

        // Your latest signature in your code: win_intr_init(DriverObject, DeviceObject, &sa_wdf_device);
        // Here we pass the WDFDEVICE pointer you want to keep.
        win_intr_init(DriverObject, wdmDevObj, &g_WdfDevice);
    }

    {
        OSSTATUS st = secagent_init();
        if (SA_FAIL(st))
        {
            // Keep device alive so IOCTLs can still be used (e.g., unload command).
            secagent_mark_not_running();
            return STATUS_SUCCESS; // legacy behavior
        }
    }

    return STATUS_SUCCESS;
}
