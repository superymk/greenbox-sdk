// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header — Limited License
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

#ifndef VK_PAL_INTERFACES_H
#define VK_PAL_INTERFACES_H

// SecApp Event Flags
//// Handled
#define SAP_EVENT_HANDLE_STATUS_INVALID		0
#define SAP_EVENT_NOT_HANDLED				1 // Valid request, but not handled
#define SAP_EVENT_HANDLING					2 // Valid request, and being handled
#define SAP_EVENT_HANDLED					3 // Valid request, and handled

//// Returned status of the response
#define SP_EVENT_SUCCESS					0


// Signal definition
//! The first 256 (0 - 255) are reserved for interrupt vectors
#define INTR_OFFSET		255
#define SIG_INVALID		(INTR_OFFSET + 1)

// Communication channel between SecOS-SecApp
#define COMM_SECOS_SECAPP_BUFSIZE	(16 << PAGE_SHIFT_4K)




#ifndef __ASSEMBLY__

/********* Required definitions from SecBase *********/
//! <cpu_idx> of VCPU in SecBase; i.e., logic processor id in SecBase. 
//! [NOTE] XMHF defines VCPU's cpu_id as lapic_id and defines <cpu_idx> as logic processor id, so SecBase uses the name
//! <cpu_idx>.
typedef uint32_t 	sb_cpu_idx_t;




/********* Signal - Info Definitions *********/
#define SIGNAL_SECOS_TIMER						10	// Reason: Timer event (interrupt) that should sent to INIT only
#define SIGNAL_CALLED_BY_APP			        101
#define SIGNAL_CALLED_BY_SECOS_SECPROCESS		102
#define SIGNAL_CALLED_BY_SECBASE		        103		
#define SIGNAL_EVENT_LIST_MAINTANCE		        104 // Reason: The pending events are too many
#define SIGNAL_REQEXIT_BY_APP			        105 // Reason: Untrusted application requests a SecProcess to exit gracefully

#define SECAPP_MAX_NUM_EVENTS					16
#define SECAPP_WARNING_IF_EVENTS_NUM_EXCEEDS	(SECAPP_MAX_NUM_EVENTS - 4)

struct _sp_msg_header
{
	uint32_t		req_id;
	uint32_t		req_response:1,
					handled:2, 		/// must be 2-bits, because using 0 here may cause errors (empty msg have 0 value)
					__reserved:29;
	size_t			data_size;
	spid_t			sender_id; 		/// the sender SecApp ID or SecOS ID (O for SecOS)
	sb_cpu_idx_t	sb_cpu_idx; 	/// If the sender is SecBase, then this field records the SecBase CPU idx of the channel
	uint32_t		ack_req_id; 	/// The request id which we response to
	uint32_t		cmd:6,			/// SecProcesses common commands are defined by SecOS, including setting up communicatio channels.
					sub_cmd:26;		/// SecProcesses specific commands are defined in SecProcesses.
	uint32_t		resp_status;	/// Returned status of the response
} __attribute__ ((packed));

#define SECPROCESS_MSG_SIZE						((size_t)(256))
#define SECPROCESS_MSG_HEADER_SIZE				((size_t)(sizeof(struct _sp_msg_header)))
#define SECPROCESS_MSG_DATA_SIZE				((size_t)(SECPROCESS_MSG_SIZE - SECPROCESS_MSG_HEADER_SIZE))


//! Message sent to SecProcesses
typedef struct
{
	struct _sp_msg_header 	header;

	uint8_t					data[SECPROCESS_MSG_DATA_SIZE];
} SP_MESSAGE;




/****** Notify SecApp about Secbase trapped untrusted accesses ******/
#define MEM_VAL_LENGTH  (64) // AVX-512 bits use 64 bytes in values

typedef struct _mem_access_info 
{
	uint8_t		retry_access;	// Do not use <bool> because mHV and SecOS may have different definiton of bool
	uint8_t		allow_write;	
	uint64_t 	mmio_dest_spa;
	uint8_t 	value[MEM_VAL_LENGTH];
    uint32_t    value_size;
} __attribute__ ((packed)) MEM_ACCESS_INFO;

typedef struct _pio_access_info 
{
	uint8_t		retry_access;	// Do not use <bool> because mHV and SecOS may have different definiton of bool
	uint8_t		allow_write;	
	uint16_t	port_dest;
	uint64_t 	value;
	uint32_t	access_size;
} __attribute__ ((packed)) PIO_ACCESS_INFO;

typedef struct _sp_comm_info 
{
	spid_t		requester;
	
	shandle_t	h_comm;
} COMM_ISP_INFO;

typedef enum _trappf_mem_access_type
{
	TRAPPED_READ = 1,
	TRAPPED_WRITE = 2,
	TRAPPED_EXECUTE = 3
} TRAPPF_MEM_ACCESS_TYPE;

typedef struct _memobj_user_access 
{
	/* SecProcess info */
	// spid_t		requester;

	/* Fault instruction */
	vaddr_t		trapped_inst;
	/// @brief ebp on x86, or rbp on x64 
	vaddr_t		fn_stack_frame_bp;
	/// @brief High address of memory object owner SecProcess's stack
	vaddr_t		owner_sp_stack_bottom;
	/// @brief Low address of memory object owner SecProcess's stack
	vaddr_t		owner_sp_stack_top;
	
	// Memory object info
	// shandle_t	h_obj;
	TRAPPF_MEM_ACCESS_TYPE	access_type;
	vaddr_t		access_vaddr;
	paddr_t		access_paddr;
	// uint64_t	value;
} MEMOBJ_USER_ACCESS_INFO;

typedef struct _intr_access_info 
{
	irq_t		irq;
	intr_t		intr;
} INTR_ACCESS_INFO;

// Commands of <SP_MESSAGE->header.cmd>
#define REQ_MHV_TRAP_MEM_READ		1
#define REQ_MHV_TRAP_MEM_WRITE		2
#define REQ_MHV_TRAP_PIO_READ 		4
#define REQ_MHV_TRAP_PIO_WRITE 		5
#define REQ_MHV_TRAP_INTR_ACCESS	10

#define REQ_SOS_INIT_COMM_ISP 		12
#define REQ_OBJ_USER_MEM_ACCESS		14
#define REQ_SP_SPECIFIC_CALL		15

/********* SecProcess Info *********/
/// @brief Reserved name of the trusted INIT SecProcess.
#define TRUSTED_SP_INIT_NAME          "INIT"

/// @brief Reserved name of the trusted DesktopMgr SecProcess.
/// [NOTE] DesktopMgr is a SecService, not a SecApp. 
#define TRUSTED_SP_DesktopMgr_NAME          "SecService-DesktopMgr"

/// @brief Reserved name of the trusted Login SecProcess.
#define TRUSTED_SP_Login_NAME               "Login"



#define uTPM_PCR_SECSRV_INIT_IMAGE                (4)
#define uTPM_PCR_SECSRV_INIT_SECURE_PARAMS        (5)
#define uTPM_PCR_IOSM_PCIE_IMAGE                (6)
#define uTPM_PCR_IOSM_PCIE_SECURE_PARAMS        (7)
#define uTPM_PCR_IOSM_PS2_IMAGE                 (8)
#define uTPM_PCR_IOSM_PS2_SECURE_PARAMS         (9)
#define uTPM_PCR_IOSM_USB2_IMAGE                (10)
#define uTPM_PCR_IOSM_USB2_SECURE_PARAMS        (11)
#define uTPM_PCR_IOSM_USB3_IMAGE                (12)
#define uTPM_PCR_IOSM_USB3_SECURE_PARAMS        (13)
#define uTPM_PCR_IOSM_FB_IMAGE                  (14)
#define uTPM_PCR_IOSM_FB_SECURE_PARAMS          (15)
#define uTPM_PCR_IOSM_GPU_IMAGE                 (16)
#define uTPM_PCR_IOSM_GPU_SECURE_PARAMS         (17)
#define uTPM_PCR_SECSERVICE_DTM_IMAGE           (18)
#define uTPM_PCR_SECSERVICE_DTM_PARAMS          (19)


/// @brief Parameters of Trusted SecProcess (ignored for untrusted SecProcesses).
typedef struct
{
	/// @brief Is this SecProcess the "Init" process? 
	/// SecOS regards the init SecProcess as a trusted process
	bool						is_init_sp;

	/// @brief Is the SecProcess schedulable? 
	/// [NOTE] Do not put its type to be bool, because the default value 0 would mean "not schedulable"
	enum sp_schedulable			sp_schedulable;

    /// @brief The uTPM's Platform Configuration Register to store the hash of the SecProcess image. 
    uint32_t                    utpm_pcr_image;

    /// @brief The uTPM's Platform Configuration Register to store the hash of the SecProcess secure parameters.
    uint32_t                    utpm_pcr_secure_params;
} TRUSTED_SP_CONST_PARAMS;

typedef struct
{
	int _reserved;
} SECAPP_CONST_PARAMS;


/// @brief 
typedef struct 
{
	uint64_t 					version_number;
	
	enum sp_type				sp_type;
	char						sp_name[SP_NAME_MAX_SIZE];

	// Flags
	
	/// @brief True iff this SecProcess a trusted process.
	/// If set, then the SecProcess loader must provide the 1st party signature of the SecProcess in the <sp_signature> 
	/// field of <SP_USTRUCT>.
	bool						is_trusted;

	// Parameters 
	union {
		TRUSTED_SP_CONST_PARAMS trusted_sp_params;
		SECAPP_CONST_PARAMS		sap_params;
	};
} SP_CONST_PARAMETERS;



/// @brief User level process block of the SecProcess
typedef struct _sp_utask
{
	/*** SecApp information ***/ 
	/// @brief ID of the SecProcess
	spid_t				sp_id;
	/// @brief Is the SecProcess the Init SecProcess?
	bool				is_init_sp;
	/// @brief ID of the INIT SecProcess
	spid_t				init_sp_id;

	/// @brief True iff the SecProcess is signed by the 1st party (i.e., signed by us (e.g., CMU).).
	bool 				is_signed_by_1stparty;

	/*** Stack ***/
	vaddr_t 			stack_bottom;
	/*** Heap ***/
	/// @brief Virtual address of the DMA heap of the SecProcess
	vaddr_t 			dma_pool;
	paddr_t				dma_pool_paddr;
	size_t 				dma_pool_size;
	shandle_t			h_dma;
	
	vaddr_t 			mem_pool;
	size_t				mem_pool_size;
	shandle_t			h_heap;

	vaddr_t 			sp_app_comm;
	size_t 				sp_app_comm_size;
	shandle_t			h_sp_app_comm;
	
	vaddr_t 			sharedmem_start;
	size_t 				sharedmem_size;
	shandle_t			h_sharedmem;

	// Remappable virtual memory region
	vaddr_t				remappable_base;

	// Event 
	//uint32_t			num_events;
	SP_MESSAGE 			events[SECAPP_MAX_NUM_EVENTS];
	//lock_t 				event_lock;

	SP_MESSAGE			secbase_event;	// [TODO] If SecOS runs on multi-cores, then this field needs to be an array.
	lock_t 				secbase_event_lock;

	/*********** SecProcess Execute Parameters (i.e., argv) **********/
	uint8_t				exec_params_secure[SP_EXEC_PARAMS_SECURE_SIZE];
	uint32_t			exec_params_secure_size;
	uint8_t				exec_params_nonsecure[SP_EXEC_PARAMS_NONSECURE_SIZE];
	uint32_t			exec_params_nonsecure_size;
} SP_UTASK;

/******************* SecProcess Functions ********************/
/// @brief SecProcess constant properties (e.g., name, type).
extern const SP_CONST_PARAMETERS sp_constant_properties;

/// @brief SecProcess's main entry at ring3.
/// @param user_sp 
extern void sp_entry(SP_UTASK* user_sp);

#endif // __ASSEMBLY__
#endif // VK_PAL_INTERFACES_H