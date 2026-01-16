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

// -----------------------
// Globals
// -----------------------
static WDFDEVICE g_WdfDevice = NULL;

// -----------------------
// Core IOCTL logic (no completion here)
// METHOD_BUFFERED => use the same in/out buffer
// -----------------------
static NTSTATUS
SaProcessIoctlBuffered(
    IN  ULONG IoControlCode,
    _Inout_updates_bytes_(BufferLen) PVOID Buffer,
    IN  size_t InLen,
    IN  size_t OutLen,
    OUT size_t* BytesWritten
    )
{
    *BytesWritten = 0;

    if (IoControlCode != IOCTL_SECAGENT) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (Buffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (InLen < sizeof(struct tz_cmd_packet) || OutLen < sizeof(struct tz_cmd_packet)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    struct tz_cmd_packet* cmd = (struct tz_cmd_packet*)Buffer;

    int copy_retval = 1;
    OSSTATUS st = ioctl_request_handler(cmd, &copy_retval);
    cmd->status = st;

    *BytesWritten = sizeof(struct tz_cmd_packet);

    // Keep your legacy behavior: if OSSTATUS indicates failure, reflect that in NTSTATUS too.
    return SA_FAIL(st) ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
}

// -----------------------
// WDF EvtIoDeviceControl => calls SaProcessIoctlBuffered
// -----------------------
static VOID
SaEvtIoDeviceControl(
    IN WDFQUEUE   Queue,
    IN WDFREQUEST Request,
    IN size_t     OutputBufferLength,
    IN size_t     InputBufferLength,
    IN ULONG      IoControlCode
    )
{
    UNREFERENCED_PARAMETER(Queue);
    PAGED_CODE();

    NTSTATUS nt;
    PVOID outBuf = NULL;
    size_t outBufLen = 0;
    PVOID inBuf = NULL;
    size_t inBufLen = 0;

    // METHOD_BUFFERED: both retrieve calls usually return the same SystemBuffer pointer.
    nt = WdfRequestRetrieveInputBuffer(Request, 0, &inBuf, &inBufLen);
    if (!NT_SUCCESS(nt)) {
        WdfRequestComplete(Request, nt);
        return;
    }

    nt = WdfRequestRetrieveOutputBuffer(Request, 0, &outBuf, &outBufLen);
    if (!NT_SUCCESS(nt)) {
        WdfRequestComplete(Request, nt);
        return;
    }

    // Use the framework-provided lengths (InputBufferLength/OutputBufferLength),
    // but also clamp to retrieved buffer sizes to be safe.
    size_t inLen  = (size_t)min((ULONG_PTR)InputBufferLength,  (ULONG_PTR)inBufLen);
    size_t outLen = (size_t)min((ULONG_PTR)OutputBufferLength, (ULONG_PTR)outBufLen);

    // For METHOD_BUFFERED we want the single in/out system buffer.
    // Prefer outBuf (same as inBuf) as the inout buffer.
    size_t bytes = 0;
    nt = SaProcessIoctlBuffered(IoControlCode, outBuf, inLen, outLen, &bytes);

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
    if (g_WdfDevice != NULL) {
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
        &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R
    );
    if (pInit == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Enforce buffered I/O because your IOCTL path expects a SystemBuffer-like buffer.
    WdfDeviceInitSetIoType(pInit, WdfDeviceIoBuffered);

    // Optional: exclusive open
    // WdfDeviceInitSetExclusive(pInit, TRUE);

    nt = WdfDeviceInitAssignName(pInit, &ntDeviceName);
    if (!NT_SUCCESS(nt)) {
        WdfDeviceInitFree(pInit);
        return nt;
    }

    // No FileCreate/FileClose: we do NOT set file object config.
    // WDF will default-complete create/close/cleanup successfully.

    // Create device
    WDF_OBJECT_ATTRIBUTES_INIT(&devAttrs);

    nt = WdfDeviceCreate(&pInit, &devAttrs, &g_WdfDevice);
    if (!NT_SUCCESS(nt)) {
        if (pInit != NULL) WdfDeviceInitFree(pInit);
        return nt;
    }

    // Create symbolic link for user-mode opens
    nt = WdfDeviceCreateSymbolicLink(g_WdfDevice, &symLinkName);
    if (!NT_SUCCESS(nt)) {
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
    if (!NT_SUCCESS(nt)) {
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
    if (!NT_SUCCESS(nt)) {
        secagent_mark_not_running();
        // Keep your legacy behavior if you insist: load succeeds even if init fails
        return STATUS_SUCCESS;
    }

    // Start WPP after WdfDriverCreate (matches sample guidance)
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    // 1) Create WDF control device
    nt = SaCreateControlDevice(wdfDriver);
    if (!NT_SUCCESS(nt)) {
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
        if (SA_FAIL(st)) {
            // Keep device alive so IOCTLs can still be used (e.g., unload command).
            secagent_mark_not_running();
            return STATUS_SUCCESS; // legacy behavior
        }
    }

    return STATUS_SUCCESS;
}
