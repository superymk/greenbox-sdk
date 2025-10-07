// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SECAPP_COMMUNICATION
#define APP_SECAPP_COMMUNICATION

#include <appddk.h>

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

#define INVALID_COMM_IPC_MESSAGE_ID		((uint32_t) -1)

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

struct ipc_msg
{
	struct ipc_header 	header;
			
	uint8_t 			req_data[IPC_MSG_DATA_SIZE];
};

// The shared information of ring buffer between SecOS and OS
// The shared information of ring buffer between SecOS and OS
struct ipc_ctl
{
	uint32_t 	p_slot; //!< producer slot: the next msg slot to produce
	uint32_t	c_slot;
}; 

#define CHAN_SAP_CTL(chan)	(volatile struct ipc_ctl*)((chan)->comm_ctl_vaddr)
#define CHAN_SAP_BUF(chan)	(volatile struct ipc_msg*)((chan)->comm_buf_vaddr)
#endif // __ASSEMBLY__


#endif // APP_SECAPP_COMMUNICATION