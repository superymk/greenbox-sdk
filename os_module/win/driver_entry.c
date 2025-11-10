// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <ntddk.h>
#include <wdf.h>

#include "driver_entry.h"
#include "win_intr.h"
#include <sa_headers.h>

static WCHAR NameBuffer[] = L"\\Device\\SecAgent";
static WCHAR DOSNameBuffer[] = L"\\DosDevices\\SecAgent";

static NTSTATUS HandleIOCTL(
    __in PDEVICE_OBJECT pDeviceObj,
    __in PIRP pIrp)
{
    PIO_STACK_LOCATION pStack;
    struct tz_cmd_packet *cmd_packet = NULL;
    struct tz_cmd_packet *in_cmd_packet = NULL;
    OSSTATUS status = EINVAL;
    int copy_retval = 1;
    uint32_t dwDataWritten = 0;

    UNREFERENCED_PARAMETER(pDeviceObj);
    assert((PASSIVE_LEVEL == KeGetCurrentIrql()), ("HandleIOCTL's entry is in non-Passive IRQL"));

    // Check: <pIrp> must be a valid pointer
    if (!pIrp)
    {
        status = EINVAL;
        goto finish;
    }

    DRV_DEBUG("<HandleIOCTL> Begin handling request.\n");

    //-f--> We get the parameters referring to our driver.
    pStack = IoGetCurrentIrpStackLocation(pIrp);

    switch (pStack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_SECAGENT:
    {
        cmd_packet = (struct tz_cmd_packet *)os->mm_malloc_zeroed(sizeof(struct tz_cmd_packet));
        if (!cmd_packet)
        {
            DRV_ERR("<cmd_packet> allocation error!\n");
            status = E_NO_MEM;
            goto finish;
        }

        in_cmd_packet = (struct tz_cmd_packet *)pIrp->AssociatedIrp.SystemBuffer;

        status = os->copy_from_user(cmd_packet, (void *)in_cmd_packet, sizeof(struct tz_cmd_packet));
        if (SA_FAIL(status))
        {
            DRV_ERR("copy_from_user error!\n");
            status = E_INTERNAL_ERROR;
            goto finish;
        }

        status = ioctl_request_handler(cmd_packet, &copy_retval);
        cmd_packet->status = status;

        // Copying the result back to the User Space
        status = os->copy_to_user((void *)in_cmd_packet, cmd_packet, sizeof(struct tz_cmd_packet));
        if (SA_FAIL(status))
        {
            DRV_ERR("copy_to_user error!\n");
            status = E_INTERNAL_ERROR;
            dwDataWritten = 0;
            goto finish;
        }
        dwDataWritten = sizeof(struct tz_cmd_packet);
    }
    default:
        break;
    }

finish:
    assert((PASSIVE_LEVEL == KeGetCurrentIrql()), ("HandleIOCTL's exit is in non-Passive IRQL"));

    if (cmd_packet)
        os->mm_free(cmd_packet);

    if (SA_FAIL(status))
        pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    else
        pIrp->IoStatus.Status = STATUS_SUCCESS;

    pIrp->IoStatus.Information = dwDataWritten;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DRV_DEBUG("<HandleIOCTL> Finish handling request.\n");
    return STATUS_SUCCESS;
}

static PDEVICE_OBJECT __RegisterDriver(IN PDRIVER_OBJECT DriverObject)
{
    NTSTATUS status;
    PDEVICE_OBJECT DeviceObject = NULL;
    UNICODE_STRING uniNameString, uniDOSString;

    RtlInitUnicodeString(&uniNameString, NameBuffer);
    RtlInitUnicodeString(&uniDOSString, DOSNameBuffer);

    status = IoCreateDevice(DriverObject,             // DriverObject
                            sizeof(DEVICE_EXTENSION), // DeviceExtensionSize
                            &uniNameString,           // DeviceName
                            FILE_DEVICE_UNKNOWN,      // DeviceType
                            0,                        // DeviceCharacteristics
                            FALSE,                    // Must be shared
                            &DeviceObject);           // *DeviceObject

    if (!NT_SUCCESS(status))
    {
        DRV_ERR("<__RegisterDriver> IoCreateDevice Error!\n");
        return NULL;
    }

    status = IoCreateSymbolicLink(&uniDOSString, &uniNameString);

    if (!NT_SUCCESS(status))
    {
        DRV_ERR("<__RegisterDriver> IoCreateSymbolicLink Error!\n");
        return NULL;
    }

    return DeviceObject;
}

static void __UnregisterDriver(void)
{
    UNICODE_STRING uniDOSString;

    // Delete Symbolic Link
    RtlInitUnicodeString(&uniDOSString, DOSNameBuffer);
    IoDeleteSymbolicLink(&uniDOSString);
}

NTSTATUS SecAgent_CreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
// ++

// Routine Description:

//     This device control dispatcher handles create & close IRPs.

// Arguments:

//     DeviceObject - Context for the activity.
//     Irp          - The device control argument block.

// Return Value:

//     NTSTATUS

// --
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;
    PFILE_CONTEXT fileContext;

    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->FileObject != NULL);

    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:
        // DbgPrintEx("IRP_MJ_CREATE\n");

        fileContext = ExAllocatePoolQuotaZero(NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                              sizeof(FILE_CONTEXT),
                                              SA_TAG);

        if (NULL == fileContext)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        IoInitializeRemoveLock(&fileContext->FileRundownLock, SA_TAG, 0, 0);

        //
        // Make sure nobody is using the FsContext scratch area.
        //
        ASSERT(irpStack->FileObject->FsContext == NULL);

        //
        // Store the context in the FileObject's scratch area.
        //
        irpStack->FileObject->FsContext = (PVOID)fileContext;

        status = STATUS_SUCCESS;
        break;

    case IRP_MJ_CLOSE:
        // DbgPrintEx("IRP_MJ_CLOSE\n");

        fileContext = irpStack->FileObject->FsContext;

        ExFreePoolWithTag(fileContext, SA_TAG);

        status = STATUS_SUCCESS;
        break;

    default:
        ASSERT(FALSE); // should never hit this
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS SecAgent_Cleanup(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)

// ++

// Routine Description:

//     This device control dispatcher handles Cleanup IRP.

// Arguments:

//     DeviceObject - Context for the activity.
//     Irp          - The device control argument block.

// Return Value:

//     NTSTATUS

// --
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;
    // PNOTIFY_RECORD      notifyRecord;
    PDEVICE_EXTENSION deviceExtension;
    PFILE_CONTEXT fileContext;

    // DRV_INFO("==>EventCleanup\n");

    deviceExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->FileObject != NULL);

    fileContext = irpStack->FileObject->FsContext;

    //
    // This acquire cannot fail because you cannot get more than one
    // cleanup for the same handle.
    //
    status = IoAcquireRemoveLock(&fileContext->FileRundownLock, Irp);
    ASSERT(NT_SUCCESS(status));

    //
    // Wait for all the threads that are currently dispatching to exit and
    // prevent any threads dispatching I/O on the same handle beyond this point.
    //
    IoReleaseRemoveLockAndWait(&fileContext->FileRundownLock, Irp);

    //
    // Finally complete the cleanup Irp
    //
    Irp->IoStatus.Status = status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    // DRV_INFO("<== EventCleanup\n");
    return status;
}

NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    PAGED_CODE();

    secagent_fini();

    win_intr_fini();

    __UnregisterDriver();

    // Delete Device
    IoDeleteDevice(DriverObject->DeviceObject);
    return STATUS_SUCCESS;
}

static WDFDEVICE sa_wdf_device;

_Use_decl_annotations_
VOID _wdf_drv_cleanup(WDFOBJECT DriverObject)
{
    // Do nothing
    UNREFERENCED_PARAMETER(DriverObject);
}

static OSSTATUS _wdf_dev_create(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status;
    WDFDRIVER driver;
    WDF_DRIVER_CONFIG drvCfg;
    WDF_OBJECT_ATTRIBUTES drvAttrs;

    WDF_DRIVER_CONFIG_INIT(&drvCfg, NULL);
    WDF_OBJECT_ATTRIBUTES_INIT(&drvAttrs);
    drvAttrs.EvtCleanupCallback = _wdf_drv_cleanup;

    status = WdfDriverCreate(DriverObject, RegistryPath, &drvAttrs, &drvCfg, &driver);
    if (!NT_SUCCESS(status)) 
	{
		PRINT_ERR_FORCE_OS("[Superymk] DriverEntry 0.1 status:0x%08X\n", status);
		return E_INTERNAL_ERROR;
	}

    // --- Create a KMDF control device so we have a WDFDEVICE parent ---
    PWDFDEVICE_INIT pInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_ALL);
    if (!pInit)
	{
		PRINT_ERR_FORCE_OS("[Superymk] DriverEntry 0.2 \n");
		return E_INTERNAL_ERROR;
	}

    UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\GreenBoxSecAgent");
    status = WdfDeviceInitAssignName(pInit, &devName);
    if (!NT_SUCCESS(status)) 
	{
		PRINT_ERR_FORCE_OS("[Superymk] DriverEntry 1 status:0x%08X\n", status);
		WdfDeviceInitFree(pInit); 
		return E_INTERNAL_ERROR; 
	}

    // Optional: set device type/characteristics here if you expose a DOS link, etc.
    status = WdfDeviceCreate(&pInit, WDF_NO_OBJECT_ATTRIBUTES, &sa_wdf_device);
    if (!NT_SUCCESS(status))
	{
		PRINT_ERR_FORCE_OS("[Superymk] DriverEntry 2 status:0x%08X\n", status);
		return E_INTERNAL_ERROR;
	}

    // Required for control devices
    WdfControlFinishInitializing(sa_wdf_device);

    // On success
    return OS_SUCCESS;
}

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath)
{
    OSSTATUS status = OS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;

    UNREFERENCED_PARAMETER(status);
    UNREFERENCED_PARAMETER(RegistryPath);
    PAGED_CODE();

    DeviceObject = __RegisterDriver(DriverObject);
    if (!DeviceObject)
        goto err;

    // Create WDF Device
    status = _wdf_dev_create(DriverObject, RegistryPath);
    if (SA_FAIL(status))
        goto err;

    win_intr_init(DriverObject, DeviceObject, &sa_wdf_device);

    status = secagent_init();
    if (SA_FAIL(status))
        goto err;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SecAgent_Cleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleIOCTL;
    DriverObject->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;

err:
    // SecAgent still needs to handle IOCTL in this case; e.g., unloading the driver on requests.
    DriverObject->MajorFunction[IRP_MJ_CREATE] = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SecAgent_Cleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleIOCTL;
    DriverObject->DriverUnload = DriverUnload;

    secagent_mark_not_running();

    // Returning STATUS_SUCCESS allows us to reload the driver, though SecAgent fails in fact.
    return STATUS_SUCCESS;
}