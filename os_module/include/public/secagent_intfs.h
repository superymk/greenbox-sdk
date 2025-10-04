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

#ifndef SECAGENT_INTFS
#define SECAGENT_INTFS

#define TZ_CLIENT_FULL_PATH_DEV_NAME "/dev/tz_client"
#define TZ_CLIENT_DEV "tz_client"

#define OS_KERN_MEMORY	1
#define OS_KERN_DMA	2

// SecAgent commands
#define CMD_SECAGENT_PRESENCE_REQ				1
#define CMD_SECBASE_INIT_ONDEMAND_PER_CPU		2
#define CMD_SECBASE_FINI_ONDEMAND_PER_CPU		3
#define CMD_KERNEL_PRINT						6
#define CMD_TEST_HYPERCALL						10
#define CMD_SECAGENT_TEST1						11
#define CMD_SECAGENT_TEST2						12
#define CMD_SECAGENT_TEST3						13
#define CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU	20		// The SecOS Loader must invoke this ioctl on each CPU as 
													// part of SecAgent initialization, so SecAgent knows the mapping 
													// between OS's cpu id to lapic id.

// Untrusted loaders share information
#define CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET		50
    #define NS_GB_COMPONENTS_NUM            (1)
    #define NS_GB_COMPONENTS_BOOTED_NUM     (2)
    #define NS_GB_COMPONENTS_BOOTING_ID     (3)

	#define NS_DISP_SCALING				    (5)
		#define WINDOW_SCALE_FACTOR 					(100)
	#define NS_DISP_WIDTH				    (6)
	#define NS_DISP_HEIGHT				    (7)
	#define NS_DTM_SECPROCESS_IS_EXIT		(10)
		#define DTM_SECPROCESS_TERMINATED				(1)
		#define DTM_SECPROCESS_NOT_TERMINATED			(2)
	#define NS_DTM_LOADER_USE_IOSM_FB	    (11)
		#define DTM_LOADER_DEFAULT_NOT_USE_IOSM_FB		(0)
		#define DTM_LOADER_USE_IOSM_FB					(1)
	#define NS_DTM_BACKEND_THREAD_FORCE_PRIORITY   (12)
		#define DTM_BACKEND_THREAD_PRIORITY_NORMAL 		(0)
		#define DTM_BACKEND_THREAD_PRIORITY_HIGHEST 	(1)
	#define NS_MAX_INFO_SELECTOR		    (64)
	#define NS_INVALID_INFO_SELECTOR	    (NS_MAX_INFO_SELECTOR)
#define CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_GET		51

#define CMD_NSDOM_SHARE_DATA_TO_ONE_NS_PROCESS_SET		52

// Untrusted loaders utility functions
#define CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE		60
    #define GLOBAL_SPINLOCK_OS_SOUND            (1)
    #define GLOBAL_SPINLOCK_MAX_SELECTOR		(4)
	#define GLOBAL_SPINLOCK_INVALID_SELECTOR	(GLOBAL_SPINLOCK_MAX_SELECTOR)
#define CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE		    61

// #define CMD_ALLOC_OS_CONTIGUOUS_MEM 			100
// #define CMD_FREE_OS_CONTIGUOUS_MEM 			101
// #define CMD_TRIGGER_OS_PENDING_INTRS			105
// #define CMD_APP_CACHE_FLUSH_RANGE				106

/// @brief Allocate and lock a memory region of <num_pages> for the S domain. The memory region is not zeroed to improve  
/// SecAgent responsiveness because the region could be very large.
/// @param cmd_packet Arg1: <num_pages> of uint32_t. Number of 4K pages to be allocated and locked.
/// @return  
#define CMD_ALLOC_AND_LOCK_PAGES_FOR_SDOM       100

/// @brief Free and unlock the memory region allocated for the S domain.
/// @param cmd_packet Arg1: <uvaddr> of uvaddr_t. The user-mode vaddr of the memory region.
/// @return 
#define CMD_FREE_AND_UNLOCK_PAGES_FOR_SDOM 	    101


#define CMD_VADDR_TO_PADDR						107
#define CMD_NOTIFY_SECPROCESS					108
#define CMD_DISABLE_LOCAL_IRQ					109
#define CMD_ENABLE_LOCAL_IRQ					110


#define CMD_POLL_OS_NOTIFICATION_INFO           111
#define CMD_REGISTER_OS_NOTIFICATION			112
#define CMD_UNREGISTER_OS_NOTIFICATION			113

// SecOS resource provisioning/management
#define CMD_TRY_ALLOC_SOS_PHYSICAL_MEM			500
#define CMD_TRY_FREE_SOS_PHYSICAL_MEM			501

// IRQ and interrupts
#define CMD_SECAGENT_X86_OS_IRQ_GET_INTR_VECTOR	600
#define CMD_OS_TRIGGER_INTR						601

// SecOS control
#define CMD_REGISTER_SECOS_PREPARE				1000	// [TODO] Ugly hack. We split this one from CMD_REGISTER_SECOS 
													// to make sure these initialization are finished before the hypercall in <CMD_REGISTER_SECOS>.
													// Later we should merge them with the <CMD_REGISTER_SECOS>.
#define CMD_REGISTER_SECOS						1001
#define CMD_UNREGISTER_SECOS					1002
#define CMD_RUN_SECOS							1003
#define CMD_TERMINATE_SECOS						1004
#define CMD_PROBE_SECOS_STATUS					1010

#define CMD_SAP_ID_ALLOCATE						1100
#define CMD_SAP_ID_DEALLOCATE					1101
#define CMD_REGISTER_SECPROCESS_MEMORY				1102
#define CMD_REGISTER_SECPROCESS						1103
// [NOTE] Unregister SecProcess is not needed, because SecProcess termination includes unregistration
#define CMD_RUN_SECPROCESS							1105
#define CMD_FORCE_TERMINATE_SECPROCESS				1106
#define CMD_SOFT_TERMINATE_SECPROCESS				1107

// SecProcess Information Query
#define CMD_SECPROCESS_GET_IDS_AND_NAMES		1150

// Trusted Display
#define CMD_TD_INIT								2000
#define CMD_TD_FINI								2001

// IOSM-USB2
#define CMD_IOSM_USB2_EHCI_PROBE				2100
#define CMD_IOSM_USB2_EHCI_UNPROBE				2101
#define CMD_IOSM_USB2_PAUSE_EHCI				2102
#define CMD_IOSM_USB2_RESUME_EHCI				2103

// IOSM-USB3
#define CMD_IOSM_USB3_SHARED_XHCI_PROBE			2110
#define CMD_IOSM_USB3_SHARED_XHCI_UNPROBE		2111
#define CMD_IOSM_USB3_SHARED_XHCI_PAUSE			2112
#define CMD_IOSM_USB3_SHARED_XHCI_RESUME				2113

// IOSM-FB
#define CMD_IOSM_FB_PROBE_GPU					2140
#define CMD_IOSM_FB_PAUSE_GPU					2141
#define CMD_IOSM_FB_RESUME_GPU					2142
#define CMD_IOSM_FB_TEST_ON						2147
#define CMD_IOSM_FB_TEST_OFF					2148

// IOSM-PS2
#define CMD_IOSM_PS2_DRAIN_BUFFERS				2150

// SecAgent signals
#define SIG_SECAPP_NOTIFICATION         		63


#ifndef __ASSEMBLY__

struct tz_cmd_packet
{
	int id;

	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;

	uint64_t status;
};



/********* For "CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET.NS_GB_COMPONENTS_BOOTING_ID" *********/
#define GB_COMPONENT_ID_SECOS       1
#define GB_COMPONENT_ID_SECSRV_INIT   2
#define GB_COMPONENT_ID_IOSM_PCIE   3
#define GB_COMPONENT_ID_IOSM_PS2    10
#define GB_COMPONENT_ID_IOSM_USB2   20
#define GB_COMPONENT_ID_IOSM_USB3   21
#define GB_COMPONENT_ID_IOSM_FB     30
   


/********* For "SecOS control" *********/
/// @brief SecOS's section addresses submitted by SecOS loader
struct sl_sos_sect_addrs
{
	// For SecOS image
	vaddr_t sos_image_start; 
  	vaddr_t sos_image_end;
	paddr_t sos_image_dest_gpa;

	// For SecOS Mem-Pool
	vaddr_t sos_mempool_start;
  	vaddr_t sos_mempool_end;
	paddr_t sos_mempool_dest_gpa;

	// For SecOS signature
	vaddr_t sos_signature_start;
	vaddr_t sos_signature_end;
	paddr_t sos_signature_dest_gpa;
};




/********* For Asynchronous Notifications from SecAgent to Untrusted Applications (e.g., SecProcess Loaders) *********/
/// @brief Applications use this struct to register their events in order to receive notifications from the SecAgent.
struct sa_app_notif_reginfo
{
	ulong_t event_obj_info;
};

//! Info associated with signals sent from SecAgent to untrusted processes.
struct sa_app_notif_info
{
    spid_t  sp_id;
    ulong_t info1;
    ulong_t info2;
};


#endif // __ASSEMBLY__

#endif // SECAGENT_INTFS   
