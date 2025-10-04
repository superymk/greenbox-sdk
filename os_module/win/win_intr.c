/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

// Modified from:
/******************************************************************/
/*  Example NT/2000/XP Interrupt & DPC Driver                     */
/*  Copyright 2001 Craig Peacock, Craig.Peacock@beyondlogic.org   */
/*  See http://www.beyondlogic.org for a complete description     */
/*  Last Updated 11th November 2001                               */
/******************************************************************/


#include "win_intr.h"

struct IO_APIC_entry os_ioapic_entries[24];

extern void Int3(void);

extern void Int20(void);
extern void Int21(void);
extern void Int22(void);
extern void Int23(void);
extern void Int24(void);
extern void Int25(void);
extern void Int26(void);
extern void Int27(void);
extern void Int28(void);
extern void Int29(void);
extern void Int2A(void);
extern void Int2B(void);
extern void Int2C(void);
extern void Int2D(void);
extern void Int2E(void);
extern void Int2F(void);

extern void Int30(void);
extern void Int31(void);
extern void Int32(void);
extern void Int33(void);
extern void Int34(void);
extern void Int35(void);
extern void Int36(void);
extern void Int37(void);
extern void Int38(void);
extern void Int39(void);
extern void Int3A(void);
extern void Int3B(void);
extern void Int3C(void);
extern void Int3D(void);
extern void Int3E(void);
extern void Int3F(void);

extern void Int40(void);
extern void Int41(void);
extern void Int42(void);
extern void Int43(void);
extern void Int44(void);
extern void Int45(void);
extern void Int46(void);
extern void Int47(void);
extern void Int48(void);
extern void Int49(void);
extern void Int4A(void);
extern void Int4B(void);
extern void Int4C(void);
extern void Int4D(void);
extern void Int4E(void);
extern void Int4F(void);

extern void Int50(void);
extern void Int51(void);
extern void Int52(void);
extern void Int53(void);
extern void Int54(void);
extern void Int55(void);
extern void Int56(void);
extern void Int57(void);
extern void Int58(void);
extern void Int59(void);
extern void Int5A(void);
extern void Int5B(void);
extern void Int5C(void);
extern void Int5D(void);
extern void Int5E(void);
extern void Int5F(void);

extern void Int60(void);
extern void Int61(void);
extern void Int62(void);
extern void Int63(void);
extern void Int64(void);
extern void Int65(void);
extern void Int66(void);
extern void Int67(void);
extern void Int68(void);
extern void Int69(void);
extern void Int6A(void);
extern void Int6B(void);
extern void Int6C(void);
extern void Int6D(void);
extern void Int6E(void);
extern void Int6F(void);

extern void Int70(void);
extern void Int71(void);
extern void Int72(void);
extern void Int73(void);
extern void Int74(void);
extern void Int75(void);
extern void Int76(void);
extern void Int77(void);
extern void Int78(void);
extern void Int79(void);
extern void Int7A(void);
extern void Int7B(void);
extern void Int7C(void);
extern void Int7D(void);
extern void Int7E(void);
extern void Int7F(void);

extern void Int80(void);
extern void Int81(void);
extern void Int82(void);
extern void Int83(void);
extern void Int84(void);
extern void Int85(void);
extern void Int86(void);
extern void Int87(void);
extern void Int88(void);
extern void Int89(void);
extern void Int8A(void);
extern void Int8B(void);
extern void Int8C(void);
extern void Int8D(void);
extern void Int8E(void);
extern void Int8F(void);

extern void Int90(void);
extern void Int91(void);
extern void Int92(void);
extern void Int93(void);
extern void Int94(void);
extern void Int95(void);
extern void Int96(void);
extern void Int97(void);
extern void Int98(void);
extern void Int99(void);
extern void Int9A(void);
extern void Int9B(void);
extern void Int9C(void);
extern void Int9D(void);
extern void Int9E(void);
extern void Int9F(void);

extern void IntA0(void);
extern void IntA1(void);
extern void IntA2(void);
extern void IntA3(void);
extern void IntA4(void);
extern void IntA5(void);
extern void IntA6(void);
extern void IntA7(void);
extern void IntA8(void);
extern void IntA9(void);
extern void IntAA(void);
extern void IntAB(void);
extern void IntAC(void);
extern void IntAD(void);
extern void IntAE(void);
extern void IntAF(void);

extern void IntB0(void);
extern void IntB1(void);
extern void IntB2(void);
extern void IntB3(void);
extern void IntB4(void);
extern void IntB5(void);
extern void IntB6(void);
extern void IntB7(void);
extern void IntB8(void);
extern void IntB9(void);
extern void IntBA(void);
extern void IntBB(void);
extern void IntBC(void);
extern void IntBD(void);
extern void IntBE(void);
extern void IntBF(void);

extern void IntC0(void);
extern void IntC1(void);
extern void IntC2(void);
extern void IntC3(void);
extern void IntC4(void);
extern void IntC5(void);
extern void IntC6(void);
extern void IntC7(void);
extern void IntC8(void);
extern void IntC9(void);
extern void IntCA(void);
extern void IntCB(void);
extern void IntCC(void);
extern void IntCD(void);
extern void IntCE(void);
extern void IntCF(void);

extern void IntD0(void);
extern void IntD1(void);
extern void IntD2(void);
extern void IntD3(void);
extern void IntD4(void);
extern void IntD5(void);
extern void IntD6(void);
extern void IntD7(void);
extern void IntD8(void);
extern void IntD9(void);
extern void IntDA(void);
extern void IntDB(void);
extern void IntDC(void);
extern void IntDD(void);
extern void IntDE(void);
extern void IntDF(void);

extern void IntE0(void);
extern void IntE1(void);
extern void IntE2(void);
extern void IntE3(void);
extern void IntE4(void);
extern void IntE5(void);
extern void IntE6(void);
extern void IntE7(void);
extern void IntE8(void);
extern void IntE9(void);
extern void IntEA(void);
extern void IntEB(void);
extern void IntEC(void);
extern void IntED(void);
extern void IntEE(void);
extern void IntEF(void);

extern void IntF0(void);
extern void IntF1(void);
extern void IntF2(void);
extern void IntF3(void);
extern void IntF4(void);
extern void IntF5(void);
extern void IntF6(void);
extern void IntF7(void);
extern void IntF8(void);
extern void IntF9(void);
extern void IntFA(void);
extern void IntFB(void);
extern void IntFC(void);
extern void IntFD(void);
extern void IntFE(void);
extern void IntFF(void);


static PDRIVER_OBJECT DriverObject = NULL;
static PDEVICE_OBJECT DeviceObject = NULL;
static intr_t sos_os_intr = INVALID_INTR;
static lapic_id_t sos_os_intr_os_lapic_id = INVALID_LAPIC_ID;

void win_int3_debug_break(void)
{
	Int3();
}

void win_intr_trigger_interrupt(uint32_t vector)
{
	switch(vector)
	{
		case 0x20:
			Int20();
			return;
		case 0x21:
			Int21();
			return;
		case 0x22:
			Int22();
			return;
		case 0x23:
			Int23();
			return;
		case 0x24:
			Int24();
			return;
		case 0x25:
			Int25();
			return;
		case 0x26:
			Int26();
			return;
		case 0x27:
			Int27();
			return;
		case 0x28:
			Int28();
			return;
		case 0x29:
			Int29();
			return;
		case 0x2A:
			Int2A();
			return;
		case 0x2B:
			Int2B();
			return;
		case 0x2C:
			Int2C();
			return;
		case 0x2D:
			Int2D();
			return;
		case 0x2E:
			Int2E();
			return;
		case 0x2F:
			Int2F();
			return;

		case 0x30:
			Int30();
			return;
		case 0x31:
			Int31();
			return;
		case 0x32:
			Int32();
			return;
		case 0x33:
			Int33();
			return;
		case 0x34:
			Int34();
			return;
		case 0x35:
			Int35();
			return;
		case 0x36:
			Int36();
			return;
		case 0x37:
			Int37();
			return;
		case 0x38:
			Int38();
			return;
		case 0x39:
			Int39();
			return;
		case 0x3A:
			Int3A();
			return;
		case 0x3B:
			Int3B();
			return;
		case 0x3C:
			Int3C();
			return;
		case 0x3D:
			Int3D();
			return;
		case 0x3E:
			Int3E();
			return;
		case 0x3F:
			Int3F();
			return;
		
		case 0x40:
			Int40();
			return;
		case 0x41:
			Int41();
			return;
		case 0x42:
			Int42();
			return;
		case 0x43:
			Int43();
			return;
		case 0x44:
			Int44();
			return;
		case 0x45:
			Int45();
			return;
		case 0x46:
			Int46();
			return;
		case 0x47:
			Int47();
			return;
		case 0x48:
			Int48();
			return;
		case 0x49:
			Int49();
			return;
		case 0x4A:
			Int4A();
			return;
		case 0x4B:
			Int4B();
			return;
		case 0x4C:
			Int4C();
			return;
		case 0x4D:
			Int4D();
			return;
		case 0x4E:
			Int4E();
			return;
		case 0x4F:
			Int4F();
			return;

		case 0x50:
			Int50();
			return;
		case 0x51:
			Int51();
			return;
		case 0x52:
			Int52();
			return;
		case 0x53:
			Int53();
			return;
		case 0x54:
			Int54();
			return;
		case 0x55:
			Int55();
			return;
		case 0x56:
			Int56();
			return;
		case 0x57:
			Int57();
			return;
		case 0x58:
			Int58();
			return;
		case 0x59:
			Int59();
			return;
		case 0x5A:
			Int5A();
			return;
		case 0x5B:
			Int5B();
			return;
		case 0x5C:
			Int5C();
			return;
		case 0x5D:
			Int5D();
			return;
		case 0x5E:
			Int5E();
			return;
		case 0x5F:
			Int5F();
			return;

		case 0x60:
			Int60();
			return;
		case 0x61:
			Int61();
			return;
		case 0x62:
			Int62();
			return;
		case 0x63:
			Int63();
			return;
		case 0x64:
			Int64();
			return;
		case 0x65:
			Int65();
			return;
		case 0x66:
			Int66();
			return;
		case 0x67:
			Int67();
			return;
		case 0x68:
			Int68();
			return;
		case 0x69:
			Int69();
			return;
		case 0x6A:
			Int6A();
			return;
		case 0x6B:
			Int6B();
			return;
		case 0x6C:
			Int6C();
			return;
		case 0x6D:
			Int6D();
			return;
		case 0x6E:
			Int6E();
			return;
		case 0x6F:
			Int6F();
			return;

		case 0x70:
			Int70();
			return;
		case 0x71:
			Int71();
			return;
		case 0x72:
			Int72();
			return;
		case 0x73:
			Int73();
			return;
		case 0x74:
			Int74();
			return;
		case 0x75:
			Int75();
			return;
		case 0x76:
			Int76();
			return;
		case 0x77:
			Int77();
			return;
		case 0x78:
			Int78();
			return;
		case 0x79:
			Int79();
			return;
		case 0x7A:
			Int7A();
			return;
		case 0x7B:
			Int7B();
			return;
		case 0x7C:
			Int7C();
			return;
		case 0x7D:
			Int7D();
			return;
		case 0x7E:
			Int7E();
			return;
		case 0x7F:
			Int7F();
			return;

		case 0x80:
			Int80();
			return;
		case 0x81:
			Int81();
			return;
		case 0x82:
			Int82();
			return;
		case 0x83:
			Int83();
			return;
		case 0x84:
			Int84();
			return;
		case 0x85:
			Int85();
			return;
		case 0x86:
			Int86();
			return;
		case 0x87:
			Int87();
			return;
		case 0x88:
			Int88();
			return;
		case 0x89:
			Int89();
			return;
		case 0x8A:
			Int8A();
			return;
		case 0x8B:
			Int8B();
			return;
		case 0x8C:
			Int8C();
			return;
		case 0x8D:
			Int8D();
			return;
		case 0x8E:
			Int8E();
			return;
		case 0x8F:
			Int8F();
			return;

		case 0x90:
			Int90();
			return;
		case 0x91:
			Int91();
			return;
		case 0x92:
			Int92();
			return;
		case 0x93:
			Int93();
			return;
		case 0x94:
			Int94();
			return;
		case 0x95:
			Int95();
			return;
		case 0x96:
			Int96();
			return;
		case 0x97:
			Int97();
			return;
		case 0x98:
			Int98();
			return;
		case 0x99:
			Int99();
			return;
		case 0x9A:
			Int9A();
			return;
		case 0x9B:
			Int9B();
			return;
		case 0x9C:
			Int9C();
			return;
		case 0x9D:
			Int9D();
			return;
		case 0x9E:
			Int9E();
			return;
		case 0x9F:
			Int9F();
			return;

		case 0xA0:
			IntA0();
			return;
		case 0xA1:
			IntA1();
			return;
		case 0xA2:
			IntA2();
			return;
		case 0xA3:
			IntA3();
			return;
		case 0xA4:
			IntA4();
			return;
		case 0xA5:
			IntA5();
			return;
		case 0xA6:
			IntA6();
			return;
		case 0xA7:
			IntA7();
			return;
		case 0xA8:
			IntA8();
			return;
		case 0xA9:
			IntA9();
			return;
		case 0xAA:
			IntAA();
			return;
		case 0xAB:
			IntAB();
			return;
		case 0xAC:
			IntAC();
			return;
		case 0xAD:
			IntAD();
			return;
		case 0xAE:
			IntAE();
			return;
		case 0xAF:
			IntAF();
			return;

		case 0xB0:
			IntB0();
			return;
		case 0xB1:
			IntB1();
			return;
		case 0xB2:
			IntB2();
			return;
		case 0xB3:
			IntB3();
			return;
		case 0xB4:
			IntB4();
			return;
		case 0xB5:
			IntB5();
			return;
		case 0xB6:
			IntB6();
			return;
		case 0xB7:
			IntB7();
			return;
		case 0xB8:
			IntB8();
			return;
		case 0xB9:
			IntB9();
			return;
		case 0xBA:
			IntBA();
			return;
		case 0xBB:
			IntBB();
			return;
		case 0xBC:
			IntBC();
			return;
		case 0xBD:
			IntBD();
			return;
		case 0xBE:
			IntBE();
			return;
		case 0xBF:
			IntBF();
			return;

		case 0xC0:
			IntC0();
			return;
		case 0xC1:
			IntC1();
			return;
		case 0xC2:
			IntC2();
			return;
		case 0xC3:
			IntC3();
			return;
		case 0xC4:
			IntC4();
			return;
		case 0xC5:
			IntC5();
			return;
		case 0xC6:
			IntC6();
			return;
		case 0xC7:
			IntC7();
			return;
		case 0xC8:
			IntC8();
			return;
		case 0xC9:
			IntC9();
			return;
		case 0xCA:
			IntCA();
			return;
		case 0xCB:
			IntCB();
			return;
		case 0xCC:
			IntCC();
			return;
		case 0xCD:
			IntCD();
			return;
		case 0xCE:
			IntCE();
			return;
		case 0xCF:
			IntCF();
			return;

		case 0xD0:
			IntD0();
			return;
		case 0xD1:
			IntD1();
			return;
		case 0xD2:
			IntD2();
			return;
		case 0xD3:
			IntD3();
			return;
		case 0xD4:
			IntD4();
			return;
		case 0xD5:
			IntD5();
			return;
		case 0xD6:
			IntD6();
			return;
		case 0xD7:
			IntD7();
			return;
		case 0xD8:
			IntD8();
			return;
		case 0xD9:
			IntD9();
			return;
		case 0xDA:
			IntDA();
			return;
		case 0xDB:
			IntDB();
			return;
		case 0xDC:
			IntDC();
			return;
		case 0xDD:
			IntDD();
			return;
		case 0xDE:
			IntDE();
			return;
		case 0xDF:
			IntDF();
			return;

		case 0xE0:
			IntE0();
			return;
		case 0xE1:
			IntE1();
			return;
		case 0xE2:
			IntE2();
			return;
		case 0xE3:
			IntE3();
			return;
		case 0xE4:
			IntE4();
			return;
		case 0xE5:
			IntE5();
			return;
		case 0xE6:
			IntE6();
			return;
		case 0xE7:
			IntE7();
			return;
		case 0xE8:
			IntE8();
			return;
		case 0xE9:
			IntE9();
			return;
		case 0xEA:
			IntEA();
			return;
		case 0xEB:
			IntEB();
			return;
		case 0xEC:
			IntEC();
			return;
		case 0xED:
			IntED();
			return;
		case 0xEE:
			IntEE();
			return;
		case 0xEF:
			IntEF();
			return;

		case 0xF0:
			IntF0();
			return;
		case 0xF1:
			IntF1();
			return;
		case 0xF2:
			IntF2();
			return;
		case 0xF3:
			IntF3();
			return;
		case 0xF4:
			IntF4();
			return;
		case 0xF5:
			IntF5();
			return;
		case 0xF6:
			IntF6();
			return;
		case 0xF7:
			IntF7();
			return;
		case 0xF8:
			IntF8();
			return;
		case 0xF9:
			IntF9();
			return;
		case 0xFA:
			IntFA();
			return;
		case 0xFB:
			IntFB();
			return;
		case 0xFC:
			IntFC();
			return;
		case 0xFD:
			IntFD();
			return;
		case 0xFE:
			IntFE();
			return;
		case 0xFF:
			IntFF();
			return;


		default:
			return;
		}
	
}

// void win_intr_local_irq_enable(void)
// {
// 	__asm{sti};
// }

// void win_intr_local_irq_disable(void)
// {
// 	__asm{sti};
// }


// NTSTATUS InterruptCreateDispatch(
//     IN PDEVICE_OBJECT pDeviceObject,
//     IN PIRP Irp
// )
// {
// 	UNREFERENCED_PARAMETER(pDeviceObject);
// 	Irp->IoStatus.Information = 0;
// 	Irp->IoStatus.Status = STATUS_SUCCESS;
// 	IoCompleteRequest(Irp, IO_NO_INCREMENT);
// 	return STATUS_SUCCESS;
// }

_Use_decl_annotations_ BOOLEAN _InterruptIsr(IN PKINTERRUPT Interrupt, IN OUT PVOID Context)
{
	PDEVICE_OBJECT t_DeviceObject = Context;

	UNREFERENCED_PARAMETER(Interrupt);

	// Top half of the ISR
	IoRequestDpc(t_DeviceObject,
	 		 t_DeviceObject->CurrentIrp,
			 NULL);

	return TRUE;
}

static VOID _InterruptDpcRoutine(IN PKDPC Dpc, PDEVICE_OBJECT pDeviceObject, IN PIRP Irp, IN PVOID Context)
{
	PDEVICE_EXTENSION DeviceExtension;
	PIRP pIrp;

	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(Irp);
	UNREFERENCED_PARAMETER(Context);

	// bottom half of the ISR
	pIrp = pDeviceObject->CurrentIrp;
	DeviceExtension = pDeviceObject->DeviceExtension;

	UNREFERENCED_PARAMETER(pIrp);
	UNREFERENCED_PARAMETER(DeviceExtension);

	SACOMM_DEBUG("<comm_sos_handle_queued_msgs> start!\n");
	comm_sos_handle_queued_msgs();
	SACOMM_DEBUG("<comm_sos_handle_queued_msgs> finished!\n");
	return;
}

#define AVAIL_INTR_START	(0x20)
#define AVAIL_INTR_END		(0xF0)

static PKINTERRUPT IntrObj = NULL;

static OSSTATUS _win_intr_register(intr_t intr, bool suppress_err_msg)
{
	NTSTATUS status;
   	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
	KAFFINITY Affinity;
   	// KIRQL Irql;

	// Check: No double registration of interrupts
	// if(DeviceExtension->InterruptObject)
	if(IntrObj)
	{
		DbgPrint("<win_intr_register> failed: Double registration!\n");
		return E_INV_ARG;
	}

	// DeviceExtension->Level = irq;
	/*DeviceExtension->Vector = DeviceExtension->Level;

   	intr = HalGetInterruptVector(Isa,						
                                           0,							
                                           DeviceExtension->Level,		
                                           DeviceExtension->Vector,	
                                           &Irql,						
                                           &DeviceExtension->Affinity);

   	DRV_DEBUG("intr = 0x%08X\n",intr);
   	DRV_DEBUG("Level = 0x%08X\n",DeviceExtension->Level);
   	DRV_DEBUG("Vector = 0x%08X\n",DeviceExtension->Vector);
   	DRV_DEBUG("Irql = 0x%08X\n",Irql);
   	DRV_DEBUG("Affinity = 0x%08X\n",DeviceExtension->Affinity); */

	// intr = ioapic_get_vector(irq);

	// Check: <intr> must be in [AVAIL_INTR_START, AVAIL_INTR_END)
 	if (!VALUE_IN_RANGE(intr, AVAIL_INTR_START, AVAIL_INTR_END)) 
 	{
		if(!suppress_err_msg)
			DRV_ERR("<win_intr_register> failed! intr:%u\n", intr);
		return E_INV_ARG;
 	}

	Affinity = KeQueryActiveProcessors();
 	IoInitializeDpcRequest(DeviceObject, _InterruptDpcRoutine);

  	status = IoConnectInterrupt(&IntrObj, // &DeviceExtension->InterruptObject, 	// InterruptObject
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
			DRV_ERR("IoConnectInterrupt Failed! intr:%u status:0x%X\n", intr, status);
		return E_INTERNAL_ERROR;
  	}

	// DriverObject->MajorFunction[IRP_MJ_CREATE] = InterruptCreateDispatch;

	return OS_SUCCESS;
}

void _win_intr_unregister(intr_t intr)
{
    PDEVICE_EXTENSION extension = DriverObject->DeviceObject->DeviceExtension;

	UNREFERENCED_PARAMETER(intr);

	// Disconnect Interrupt
	if(IntrObj)
	{
		// [TODO][Issue 104] When XMHF runs, IoDisconnectInterrupt hangs the Dell 7050 PC but works fine on Qemu
		// // IoDisconnectInterrupt(extension->InterruptObject);
		// IoDisconnectInterrupt(IntrObj);
		// IntrObj = NULL;
	}
}

void win_intr_init(PDRIVER_OBJECT in_DriverObject, PDEVICE_OBJECT in_DeviceObject)
{
	DriverObject = in_DriverObject;
	DeviceObject = in_DeviceObject;
}

void win_intr_fini(void)
{
	DriverObject = NULL;
	DeviceObject = NULL;
}

// int win_find_sos_os_irq(void)
// {
// 	OSSTATUS status = SUCCESS;
// 	int irq = 0;

// 	FOREACH_S(irq, 23, 23, 0, 1)
// 	{
// 		status = win_intr_register(irq);
// 		if(SA_PASS(status))
// 		{
// 			_win_intr_unregister(irq);
// 			return irq;
// 		}
// 	}

// 	return INVALID_IRQ;
// }

void win_os_intr_remove_affinity(void)
{
	OSSTATUS status = OS_SUCCESS;

	status = ioapic_remove_allintrs_affinity(1); // Remove the interrupt affinity from CPU 1
	if(SA_FAIL(status))
	{
		DRV_ERR("Failed to remove affinity in IOAPIC of CPU 1\n");
	}
}

void win_os_intr_restore_affinity(void)
{
	// Do nothing
}

static uint32_t _win_os_dedicated_cpu_get_id(void)
{
	return 0;
}

OSSTATUS win_sos_os_intr_probe_and_register(void)
{
	OSSTATUS status = OS_SUCCESS;
	intr_t result = INVALID_INTR;
	bool success = false;
	uint32_t os_cpu_id = 0;

	if(sos_os_intr != INVALID_INTR)
	{
		// DRV_ERR("Probe SecOS-OS IRQ Error! Double probe without unregistering the previous SecOS-OS interrupt vector\n"); 
		// return E_INV_ARG;

		// [TODO][Issue 104] Due to Issue 104, SecAgent only invokes IoDisconnectInterrupt during DriverUnload
		return OS_SUCCESS;
	}

	// Probe available interrupt vectors
	FOREACH_S(result, AVAIL_INTR_END, INTR_VECTOR_MAX, AVAIL_INTR_START, 1)
	{
		status = _win_intr_register(result, true);
		if(SA_PASS(status))
		{
			// Probe success!
			success = true;
			break;
		}
	}

	if(!success)
	{
		DRV_ERR("Probe SecOS-OS IRQ Error!\n"); 
		return E_INTERNAL_ERROR;
	}

	// On success
	sos_os_intr = result;

	os_cpu_id = _win_os_dedicated_cpu_get_id();

	if (!__x86_os_cpu_lapic_mapping_find(os_cpu_id, &sos_os_intr_os_lapic_id)) {
		DRV_ERR("Probe SecOS-OS IRQ Error! Cannot find LAPIC ID of the OS's CPU:%u\n", os_cpu_id);
		return E_INTERNAL_ERROR;
	}

	return OS_SUCCESS;
}

void win_sos_os_intr_unregister(void)
{
    if(sos_os_intr != INVALID_INTR)
	{
		_win_intr_unregister(sos_os_intr);
		sos_os_intr = INVALID_INTR;
	}
}

intr_t win_sos_os_intr_get_vector(void)
{
	return sos_os_intr;
}

#define TIMER_INTERRUPT_VECTOR_WINDOWS      (0xD1)
intr_t win_intr_timer(void)
{
	return TIMER_INTERRUPT_VECTOR_WINDOWS;
}

lapic_id_t win_sos_os_intr_get_os_lapic_id(void)
{
	return sos_os_intr_os_lapic_id;
}

void win_debug_intr_get_current_irq_priority(void)
{
	KIRQL irql;
	uint32_t tpr, ppr;

	// Windows IRQL
	irql = KeGetCurrentIrql();
	DRV_INFO("<win_get_current_irq_priority> Current IRQL:%u\n", irql);
	
	// Task Priority Register
	tpr = lapic_read_tpr();

	// Processor-Priority Register
	ppr = lapic_read_ppr();

	// Report
	DRV_INFO("<win_get_current_irq_priority> Current IRQL:%u, TPR:%u, PPR:%u\n", irql, tpr, ppr);
}