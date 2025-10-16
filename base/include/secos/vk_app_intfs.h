// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header - Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
 */

/// @file
/// The interface between SecOS and AppLoader
/// AppLoader is the untrusted user level code to start the SecApp(s). 
/// [NOTE] Untrusted OS/Apps may have different definition of data types (e.g., size_t, bool) than SecOS due to different
/// headers. Thus, the definitions in this file are either use our own data types (e.g., vaddr_t) or use data types that
/// have the same definitions across all untrusted OSes (e.g., uint32_t)

#ifndef _VK_APP_INTFS_H
#define _VK_APP_INTFS_H

/// data structure provided by OS to PAL and SecOS
//#define MAX_TP_PATH_USBNODES 5



// SecOS return status






// Signals of SecOS to untrusted processes
#define SOS_SIGNAL_INFO1_SP_INITIALIZED			1		 // In this case, INFO2, INFO3 and INFO4 are always 0
#define SOS_SIGNAL_INFO1_SP_NORMAL_EXIT         11       // In this case, INFO2 is the exit status, INFO3 and INFO4 are always 0
#define SOS_SIGNAL_INFO1_SP_FORCE_EXIT          12       // In this case, INFO2, INFO3 and INFO4 are always 0
#define SOS_SIGNAL_INFO1_SP_EXCEPTION_EXIT      13       // In this case, INFO2 is the exception vector, INFO3 and INFO4 are always 0

#ifndef __ASSEMBLY__

/// USB trusted path (TP) device configurations
/*struct usb_conf
{
	uint32_t 	bus_num; // USB bus number (index)
	uint32_t 	path_len; 

	// USB path from host controller (hc) to the assigned USB device
	uint32_t 	path[MAX_TP_PATH_USBNODES]; 

	// The corresponding downstream ports of the USB path
	uint32_t 	devport[MAX_TP_PATH_USBNODES]; 
}; 

/// Trusted Display (TD) related configuration 
struct td_conf
{
	// Trusted Display - Intel GPUs Related
	vaddr_t 	surface_vaddr;
	size_t		surface_size;
	uint32_t 	disp_height;
	uint32_t 	disp_width;
};

union dev_conf
{
	//struct usb_conf 	usb_conf;
	struct td_conf 		td_conf;
};

/// Trusted Path devices related configurations
// [TODO][Ticket87] The hierarchy design in <tp_devices> is not best. To make it better, 
// we should make <tp_devices> abstract (disgard PCI), and put necessary
// information to initialize each level of Bus subsystems (include PCI) and
// the device. Thus, it forms a chain, e.g. subsystem->subsystem->subsystem
// ->device. Further, we should support arbitary length of the chain
typedef struct _device_reg_info
{
	/// The bus (e.g., PCI bus) that the device connects as a peripheral
	BUS_TYPE		bus_type;
	
	TPDEV_TYPE		dev_type; // should be exclusively assigned to SecApp or shared to SecApp?
							// GPU is a special device type. 
	uint32_t  		dev_sub_type; // e.g. usb keyboard belongs to USB type

	uint32_t 		os_intr_affinity;
	
	// PCI related
	uint16_t 		pci_bdf;
	uint32_t		pci_irq;

	union dev_conf	dev_conf;
} DEVICE_REG_INFO; */


/********* SecOS Signature Section *********/ 
#define SECOS_SIGNATURE_LENGTH	(512)
#define SECPROCESS_SIGNATURE_LENGTH	(512)

typedef struct 
{
	uint8_t signature[SECOS_SIGNATURE_LENGTH];
} SECOS_SIGNATURE;




/********* SecProcess Information *********/ 
struct sp_addrs_info  
{
	vaddr_t 	sp_scode_start; // For PAL code
  	vaddr_t 	sp_scode_end;
	paddr_t		sp_scode_dest_gpa;
	vaddr_t		sp_scode_dest_gva;
	
	vaddr_t 	sp_sdata_start; // For PAL data
  	vaddr_t 	sp_sdata_end;
	paddr_t 	sp_sdata_dest_gpa;
	vaddr_t		sp_sdata_dest_gva;

	vaddr_t 	sp_rodata_start; // For PAL rodata
  	vaddr_t 	sp_rodata_end;
	paddr_t 	sp_rodata_dest_gpa;
	vaddr_t 	sp_rodata_dest_gva;

	// SecProcess params
	vaddr_t 	sp_params_start; // For PAL params
  	vaddr_t 	sp_params_end;
	paddr_t 	sp_params_dest_gpa;
	vaddr_t 	sp_params_dest_gva;

	// SecProcess stack
	vaddr_t 	sp_stack_start; // For PAL stack
  	vaddr_t 	sp_stack_end;
	paddr_t 	sp_stack_dest_gpa;
	vaddr_t 	sp_stack_dest_gva;

	// SecProcess memory
	vaddr_t 	sp_mempool_start; // For PAL mem-pool
  	vaddr_t 	sp_mempool_end;
	paddr_t 	sp_mempool_dest_gpa;
	vaddr_t 	sp_mempool_dest_gva;

	// SecProcess-App communication channel
	vaddr_t 	sp_comm_start; // For PAL mem-pool
  	vaddr_t 	sp_comm_end;
	paddr_t 	sp_comm_dest_gpa;
	vaddr_t 	sp_comm_dest_gva;

	// SecProcess shared memory
	vaddr_t 	sp_sharemem_start;
  	vaddr_t 	sp_sharemem_end;
	paddr_t 	sp_sharemem_dest_gpa;
	vaddr_t 	sp_sharemem_dest_gva;

	// SecProcess DMA
	vaddr_t 	sp_dmapool_start;
  	vaddr_t 	sp_dmapool_end;
	paddr_t 	sp_dmapool_dest_gpa;
	vaddr_t 	sp_dmapool_dest_gva;

	// SecProcess remappable virtual memory region
	vaddr_t		sp_remappable_dest_gva;
};

/// Info of the SecProcess
typedef struct _sp_user_struct
{
	/// @brief The red application booting this SecProcess
	app_pid_t					app_pid;
	/// @brief The process ID of this SecProcess
	spid_t						sp_id;

	struct sp_addrs_info  		sp_addrs;

	/*********** SecProcess Execute Parameters (i.e., argv) **********/
	/// @brief Security-sensitive execute parameters of the SecProcess. GreenBox measures this memory region.
	uint8_t						sp_exec_params_secure[SP_EXEC_PARAMS_SECURE_SIZE];
	uint32_t					sp_exec_params_secure_size;

	/// @brief Non-sensitive execute parameters of the SecProcess. GreenBox does not measure this memory region.
	uint8_t						sp_exec_params_nonsecure[SP_EXEC_PARAMS_NONSECURE_SIZE];
	uint32_t					sp_exec_params_nonsecure_size;

	/// @brief True (i.e., non-zero) iff the SecProcess presents its signature signed by us (e.g., CMU).
	uint8_t						sp_has_1stparty_signature;
	uint8_t						sp_signature[SECPROCESS_SIGNATURE_LENGTH];
	uint32_t					sp_signature_size;
} SP_USTRUCT;


extern void secos_entry(void) __attribute__((section(".text.entry")));

#endif // __ASSEMBLY__
#endif // _VK_APP_INTFS_H