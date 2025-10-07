// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IPC_COMM_OPS
#define IPC_COMM_OPS

#define INVALID_COMM_IPC_MESSAGE_ID		((uint32_t) -1)

typedef uint64_t perf_counter_t;
typedef unsigned long vaddr_t;

struct comm_ipc_channel
{	
	// memory of the channel
	vaddr_t 			comm_ctl_vaddr; //!< The vaddr of the shared communication 
								//!< control block
	vaddr_t 			comm_buf_vaddr; //!< The vaddr of the shared communication 
								//!< buffer 

	// Information private to SecOS only
	_LOCK_T 			chan_lock;
	
	// Perf statics 
	perf_counter_t 		counter_msgs; //!< The number of requests received

	// Request Info
	uint32_t			next_msg_id;
};

struct comm_secapp
{
	// Perf statics
	perf_counter_t comm_os_ipis; //!< The number of this communication channel invoked

	struct comm_ipc_channel send_chan; //!< Information of the SecOS-->OS channel
	struct comm_ipc_channel recv_chan; //!< Information of the OS-->SecOS channel
};

#endif // IPC_COMM_OPS