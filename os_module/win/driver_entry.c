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
	__in PDEVICE_OBJECT   pDeviceObj,
	__in PIRP             pIrp
)
{
	PIO_STACK_LOCATION      pStack;
	struct tz_cmd_packet*   cmd_packet = NULL;
	struct tz_cmd_packet*   in_cmd_packet = NULL;
	OSSTATUS status = EINVAL;
	int copy_retval = 1;
	uint32_t dwDataWritten = 0;

	UNREFERENCED_PARAMETER(pDeviceObj);
	assert((PASSIVE_LEVEL ==  KeGetCurrentIrql()), ("HandleIOCTL's entry is in non-Passive IRQL"));

	// Check: <pIrp> must be a valid pointer
    if(!pIrp)
	{
        status = EINVAL;
		goto finish;
	}

	DRV_DEBUG("<HandleIOCTL> Begin handling request.\n");

	//-f--> We get the parameters referring to our driver.
	pStack = IoGetCurrentIrpStackLocation(pIrp);

	switch(pStack->Parameters.DeviceIoControl.IoControlCode)
	{
		 case IOCTL_SECAGENT:
		 {
			cmd_packet = (struct tz_cmd_packet*) os->mm_malloc_zeroed(sizeof(struct tz_cmd_packet));
			if(!cmd_packet){
				DRV_ERR("<cmd_packet> allocation error!\n");
				status = E_NO_MEM;
				goto finish;
			}

			in_cmd_packet = (struct tz_cmd_packet*)pIrp->AssociatedIrp.SystemBuffer;

			status = os->copy_from_user(cmd_packet, (void*) in_cmd_packet, sizeof(struct tz_cmd_packet)); 
			if (SA_FAIL(status)) {
				DRV_ERR("copy_from_user error!\n");
				status = E_INTERNAL_ERROR;
				goto finish;
			}

			status = ioctl_request_handler(cmd_packet, &copy_retval);
			cmd_packet->status = status;

			// Copying the result back to the User Space
			status = os->copy_to_user((void*) in_cmd_packet, cmd_packet, sizeof(struct tz_cmd_packet)); 
			if (SA_FAIL(status)) {
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
	assert((PASSIVE_LEVEL ==  KeGetCurrentIrql()), ("HandleIOCTL's exit is in non-Passive IRQL"));

	if(cmd_packet)
    	os->mm_free(cmd_packet);

	if(SA_FAIL(status))
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

    status = IoCreateDevice(DriverObject,			        // DriverObject
				sizeof(DEVICE_EXTENSION),	// DeviceExtensionSize
				&uniNameString,			// DeviceName
				FILE_DEVICE_UNKNOWN,		// DeviceType
				0,				// DeviceCharacteristics
				FALSE,				// Must be shared
				&DeviceObject);			// *DeviceObject

	if(!NT_SUCCESS(status)) 
	{
		DRV_ERR("<__RegisterDriver> IoCreateDevice Error!\n");
		return NULL;
	}

	status = IoCreateSymbolicLink (&uniDOSString, &uniNameString);

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
	IoDeleteSymbolicLink (&uniDOSString);
}


NTSTATUS SecAgent_CreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)
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
    NTSTATUS            status;
    PFILE_CONTEXT       fileContext;

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

            if (NULL == fileContext) {
                status =  STATUS_INSUFFICIENT_RESOURCES;
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
            irpStack->FileObject->FsContext = (PVOID) fileContext;

            status = STATUS_SUCCESS;
            break;

        case IRP_MJ_CLOSE:
            // DbgPrintEx("IRP_MJ_CLOSE\n");

            fileContext = irpStack->FileObject->FsContext;

            ExFreePoolWithTag(fileContext, SA_TAG);

            status = STATUS_SUCCESS;
            break;

        default:
            ASSERT(FALSE);  // should never hit this
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
    PIRP Irp
    )

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
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status ;
    // PNOTIFY_RECORD      notifyRecord;
    PDEVICE_EXTENSION   deviceExtension;
    PFILE_CONTEXT       fileContext;

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

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
	PDEVICE_OBJECT DeviceObject = NULL;
	OSSTATUS status = OS_SUCCESS;

	// UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(status);
	// UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	PAGED_CODE();

	DeviceObject = __RegisterDriver(DriverObject);
	if(!DeviceObject)
		goto err;
	
	win_intr_init(DriverObject, DeviceObject);
	
	status = secagent_init();
	if(SA_FAIL(status))
		goto err;

	DriverObject->MajorFunction[IRP_MJ_CREATE]          = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SecAgent_CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = SecAgent_Cleanup;
  	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] 	= HandleIOCTL;
	DriverObject->DriverUnload 							= DriverUnload;

	return STATUS_SUCCESS;


err:
	// SecAgent still needs to handle IOCTL in this case; e.g., unloading the driver on requests.
	DriverObject->MajorFunction[IRP_MJ_CREATE]          = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SecAgent_CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = SecAgent_Cleanup;
  	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] 	= HandleIOCTL;
	DriverObject->DriverUnload 							= DriverUnload;

	secagent_mark_not_running();

	// Returning STATUS_SUCCESS allows us to reload the driver, though SecAgent fails in fact.
	return STATUS_SUCCESS;
}







/*NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    PAGED_CODE();

    DbgPrint("Bye World!");
    //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bye World!"));

    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath)
{
    PDEVICE_OBJECT DeviceObject = NULL;
	OSSTATUS status = OS_SUCCESS;

	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(status);
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	PAGED_CODE();

    DbgPrint("Hello World!");
    //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Hello World!"));

    // DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE]          = SecAgent_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SecAgent_CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = SecAgent_Cleanup;
  	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] 	= HandleIOCTL;
	DriverObject->DriverUnload 							= DriverUnload;

    return STATUS_SUCCESS;
}*/




// // [TODO][Issue 104] When XMHF runs, IoDisconnectInterrupt hangs the Dell 7050 PC but works fine on Qemu
// #define IRQ_VECTOR 80
// #define IRQ_LEVEL 2

// BOOLEAN InterruptHandler(IN PKINTERRUPT Interrupt, IN PVOID Context)
// {
//     PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)Context;
//     IoRequestDpc(devExt->DeviceObject, NULL, NULL);
//     return TRUE;
// }

// VOID DpcRoutine(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
// {
//     // Your bottom half ISR code here
// }

// NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
// {
//     UNREFERENCED_PARAMETER(RegistryPath);

//     NTSTATUS status;
//     PDEVICE_OBJECT deviceObject;
//     PDEVICE_EXTENSION devExt;

//     // Create a device object for our driver
//     status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);

//     if (!NT_SUCCESS(status)) {
// 		DbgPrint("[Superymk] <DriverEntry> 1\n");
//         return status;
//     }

//     devExt = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;
//     devExt->DeviceObject = deviceObject;




// 	KAFFINITY				Affinity;

// 	Affinity = KeQueryActiveProcessors();
//  	IoInitializeDpcRequest(deviceObject, DpcRoutine);

// 	status = IoConnectInterrupt(&devExt->InterruptObject, 	// InterruptObject
//   					    InterruptHandler,	   	// ServiceRoutine
//   					    devExt,		// ServiceContext
//  					    NULL,			// SpinLock
// 					    (ULONG)IRQ_VECTOR,		// Vector
//   					    PASSIVE_LEVEL,			// Irql
// 						PASSIVE_LEVEL,			// SynchronizeIrql
// 					    Latched,			// InterruptMode
//   					    FALSE,			// ShareVector
// 					    Affinity,	// ProcessorEnableMask
// 					    FALSE);			// FloatingSave

//   	if (!NT_SUCCESS (status)) 
//   	{
// 		DbgPrint("IoConnectInterrupt Failed! intr:%u status:0x%X\n", IRQ_VECTOR, status);
// 		return status;
//   	}

// 	DriverObject->DriverUnload 							= DriverUnload;

// 	DbgPrint("[Superymk] <DriverEntry> Succeed!\n");

//     return STATUS_SUCCESS;
// }

// NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject)
// {
//     PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;

// 	IoDisconnectInterrupt(devExt->InterruptObject);

//     // Delete the device object
//     IoDeleteDevice(DriverObject->DeviceObject);

// 	DbgPrint("[Superymk] <DriverUnload> Succeed!\n");
// 	return STATUS_SUCCESS;
// }
// // [TODO][Issue 104] End





/*
static PDEVICE_OBJECT DeviceObject = NULL;
static intr_t sos_os_intr = INVALID_INTR;

#define AVAIL_INTR_START	(0x20)
#define AVAIL_INTR_END		(0xF0)


VOID _InterruptDpcRoutine(IN PKDPC Dpc, PDEVICE_OBJECT pDeviceObject, IN PIRP Irp, IN PVOID Context);
_Use_decl_annotations_ BOOLEAN _InterruptIsr(IN PKINTERRUPT Interrupt, IN OUT PVOID Context);

static OSSTATUS _win_intr_register(intr_t intr, bool suppress_err_msg)
{
	NTSTATUS status;
   	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
	KAFFINITY Affinity;
   	// KIRQL Irql;

	// Check: No double registration of interrupts
	if(DeviceExtension->InterruptObject)
	{
		DbgPrint("<win_intr_register> failed: Double registration!\n");
		return E_INV_ARG;
	}

	// Check: <intr> must be in [AVAIL_INTR_START, AVAIL_INTR_END)
 	if (!VALUE_IN_RANGE(intr, AVAIL_INTR_START, AVAIL_INTR_END)) 
 	{
		if(!suppress_err_msg)
			DbgPrint("<win_intr_register> failed! intr:%u\n", intr);
		return E_INV_ARG;
 	}

	Affinity = KeQueryActiveProcessors();
 	IoInitializeDpcRequest(DeviceObject, _InterruptDpcRoutine);

  	status = IoConnectInterrupt(&DeviceExtension->InterruptObject, 	// InterruptObject
  					    _InterruptIsr,	   	// ServiceRoutine
  					    DeviceObject,		// ServiceContext
 					    NULL,			// SpinLock
					    (ULONG)intr,		// Vector
  					    PASSIVE_LEVEL,			// Irql
						PASSIVE_LEVEL,			// SynchronizeIrql
					    Latched,			// InterruptMode
  					    FALSE,			// ShareVector
					    Affinity,	// ProcessorEnableMask
					    FALSE);			// FloatingSave

  	if (!NT_SUCCESS (status)) 
  	{
		if(!suppress_err_msg)
			DbgPrint("IoConnectInterrupt Failed! intr:%u status:0x%X\n", intr, status);
		return E_INTERNAL_ERROR;
  	}

	DbgPrint("[Superymk] IoConnectInterrupt Succeed! intr:%u status:0x%X, DeviceExtension->InterruptObject:0x%llX\n", 
		intr, status, (uint64_t)DeviceExtension->InterruptObject);

	return OS_SUCCESS;
}

static void _win_intr_unregister(intr_t intr)
{
	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

	UNREFERENCED_PARAMETER(intr);

	// Disconnect Interrupt
	DbgPrint("[Superymk] <_win_intr_unregister> DeviceExtension->InterruptObject:0x%llX\n", (uint64_t)DeviceExtension->InterruptObject);
	IoDisconnectInterrupt(DeviceExtension->InterruptObject);
	DeviceExtension->InterruptObject = NULL;
}

static OSSTATUS win_sos_os_intr_probe_and_register(void)
{
	OSSTATUS status = OS_SUCCESS;
	intr_t result = INVALID_INTR;
	bool success = false;
	uint32_t os_cpu_id = 0;

	if(sos_os_intr != INVALID_INTR)
	{
		DbgPrint("Probe SecOS-OS IRQ Error! Double probe without unregistering the previous SecOS-OS interrupt vector\n"); 
		return E_INV_ARG;
	}

	// Probe available interrupt vectors
	FOREACH_S(result, AVAIL_INTR_END, INTR_VECTOR_MAX, AVAIL_INTR_START, 1)
	{
		status = _win_intr_register(result, false);
		if(SA_PASS(status))
		{
			// Probe success!
			success = true;
			break;
		}
	}

	if(!success)
	{
		DbgPrint("Probe SecOS-OS IRQ Error!\n"); 
		return E_INTERNAL_ERROR;
	}

	// On success
	sos_os_intr = result;

	// [TODO] currently interrupt handler hardcoded to CPU 3
	os_cpu_id = 3;

	// if (!__x86_os_cpu_lapic_mapping_find(os_cpu_id, &sos_os_intr_os_lapic_id)) {
	// 	DbgPrint("Probe SecOS-OS IRQ Error! Cannot find LAPIC ID of the OS's CPU:%u\n", os_cpu_id);
	// 	return E_INTERNAL_ERROR;
	// }

	return OS_SUCCESS;
}

static void win_sos_os_intr_unregister(void)
{
    if(sos_os_intr != INVALID_INTR)
	{
		_win_intr_unregister(sos_os_intr);
		sos_os_intr = INVALID_INTR;
	}
}







//static PDEVICE_OBJECT DeviceObject = NULL;
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    // PDEVICE_EXTENSION devExt;

    // Create a device object for our driver
    status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);

    if (!NT_SUCCESS(status)) {
		DbgPrint("[Superymk] <DriverEntry> 1\n");
        return status;
    }

	DeviceObject = deviceObject;
	{
		win_sos_os_intr_probe_and_register();
	}


	// {
	// 	// win_intr_init(DriverObject, deviceObject);

	// 	// Initialize required OS functions
	// 	// os_init();

	// 	win_sos_os_intr_probe_and_register(DeviceObject);
	// }

	DriverObject->DriverUnload 							= DriverUnload;

	DbgPrint("[Superymk] <DriverEntry> Succeed!\n");

    return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	{
		win_sos_os_intr_unregister();
	}

	// {
	// 	win_sos_os_intr_unregister(DeviceObject);
	// 	// win_intr_fini();
	// }

    // Delete the device object
    IoDeleteDevice(DriverObject->DeviceObject);

	DbgPrint("[Superymk] <DriverUnload> Succeed!\n");
	return STATUS_SUCCESS;
}
*/