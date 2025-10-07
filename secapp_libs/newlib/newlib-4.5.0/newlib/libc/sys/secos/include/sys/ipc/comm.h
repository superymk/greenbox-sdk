// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IPC_COMM
#define IPC_COMM

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

// Direction
#define DIR_SECAPP_TO_APP		1
#define DIR_APP_TO_SECAPP		2

// Handled
#define MSG_EMPTY			0
#define MSG_HANDLED			1
#define MSG_NOT_HANDLED		2

// Command Type
#define RESPONSE			1
#define SHAREDMEM_OPS		2
#define LIBC_OPS			3
#define HID_BRIDGE_OPS		4
#define GUI_BRIDGE_OPS		5
#define DESKTOP_MGR_OPS		6
#define TPM_OPS             7

// Sharedmem Ops
#define SHAREMEM_ALLOCATE	1
#define SHAREMEM_FREE		2

#ifndef __ASSEMBLY__

// [TODO] Move it to a better place
typedef unsigned long vaddr_t;

//! Ring buffer protocol for SecApp-App channel
struct ipc_header
{
	uint32_t	req_id;
	uint32_t	req_response:1,
				direction:2, //! must be 2-bits, because using 0 here may cause errors (empty msg have 0 value)
				handled:2, //! must be 2-bits, because using 0 here may cause errors (empty msg have 0 value)
				__reserved:27;
	uint32_t	data_size; 
	uint32_t	ack_req_id; //! The request id which we response to
	uint32_t	cmd:28,
				cmd_type:4;
} __attribute__ ((packed));

#define IPC_MSG_SIZE			((uint32_t)1024)										//! 1024 bytes for each message 
#define IPC_MSG_HEADER_SIZE		(sizeof(struct ipc_header)) 
#define IPC_MSG_DATA_SIZE 		(IPC_MSG_SIZE - IPC_MSG_HEADER_SIZE)
#define IPC_CHAN_SIZE			(28 + 4) * 1024							//! 32KB for each channel, 4KB for the chan ctl
#define IPC_CHAN_NUM_MSG_SLOTS	(28) 						//! 28 KB for each chan buffer, 1024 bytes for each message

//! Memory size of a SecProcess-App communication channel. The size doubles IPC_CHAN_SIZE because the channels are
// bi-directional
#define SAP_APP_COMM_SIZE		(IPC_CHAN_SIZE * 2)


struct ipc_msg
{
	struct ipc_header 	header;
			
	uint8_t 			req_data[IPC_MSG_DATA_SIZE];
};

// The shared information of ring buffer between SecOS and OS
struct ipc_ctl
{
	uint32_t 	p_slot; //!< producer slot: the next msg slot to produce
	uint32_t	c_slot;
}; 

#define CHAN_SAP_CTL(chan)	(volatile struct ipc_ctl*)((chan)->comm_ctl_vaddr)
#define CHAN_SAP_BUF(chan)	(volatile struct ipc_msg*)((chan)->comm_buf_vaddr)

/// @brief Wait for infinite time
#define COMM_WAIT_INFINITE (INT32_MAX)

/// @brief Create a message to be sent to the current SecProcess's loader red application.
/// @param req_response True if the message expects a response from the red application. Otherwise false.
/// @return If succeed, return the created new message in heap. Otherwise, return NULL.
extern struct ipc_msg* sp_comm_app_create_msg(bool req_response);

//! Issue the IPC message, but does not wait
extern bool sp_comm_app_send_msg_nowait(struct ipc_msg* msg, size_t data_size);

/// @brief Issue the IPC message, and block wait
/// @param request 
/// @param data_size 
/// @param ms Timeout in micro-seconds.
/// @return This function allocates and returns the response message on success. Otherwise, the function returns NULL.
extern struct ipc_msg* sp_comm_app_send_msg_and_wait(struct ipc_msg* request, size_t data_size, uint32_t microsecs);

extern void sp_comm_app_free_msg(struct ipc_msg* req_msg, struct ipc_msg* resp_msg);

/// @brief Initialize the communication channel to the current SecProcess's loader red application over the shared memory
/// <shared_mem_vaddr>.
/// @param shared_mem_vaddr Shared memory between the current SecProcess and its loader red application. 
extern void sp_comm_app_init(vaddr_t shared_mem_vaddr);
extern void sp_comm_app_fini(void);


typedef int32_t smem_offset_t;
#define INVALID_SMEM_OFFSET				(-1)

#define SMEM_OFFSET_TO_VADDR(offset)  	((vaddr_t)sp_get_sharedmem_base() + (offset))
#define SMEM_VADDR_TO_OFFSET(vaddr)		(((vaddr_t)(vaddr)) - sp_get_sharedmem_base())

/// @brief Request the current SecProcess's loader red application allocate memory from the shared memory.
/// @param size the size of the memory to be allocated.
/// @return If succeed, return the offset in the shared memory. Otherwise, return INVALID_SMEM_OFFSET.
extern smem_offset_t sp_comm_app_alloc_sharedmem(size_t size);

/// @brief Request the current SecProcess's loader red application to free the memory offset <offset> in the shared memory.
/// @param offset the offset of the shared memory to be freed.
/// @return If succeed, return true. Otherwise, return false.
extern bool sp_comm_app_free_sharedmem(smem_offset_t offset);

/// @brief Force the current SecProcess to process all messages received from red applications.
/// @param  
extern void sp_comm_app_process_all_app_msgs(void);

/// @brief Return the base of the SecProcess-App shared memory.
/// @param  
/// @return 
extern void* sp_get_sharedmem_base(void);

#endif //  __ASSEMBLY__
#endif // IPC_COMM