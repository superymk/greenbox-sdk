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

//! @file
//! @brief SecOS - OS Communication Protocol
//!
//! It is a two-way asyncronous protocol for multi-core case. Any party can 
//! initiate communication with the other party. The protocol is async to 
//! maximize the performance and uses ring buffer design for this purpose. 

#ifndef VK_OS_CHANNEL_H
#define VK_OS_CHANNEL_H

// Direction
#define DIR_TO_S				0 //!< Direction: Request to Secure code (SecOS/SecApps)
#define DIR_TO_NS				1 //!< Direction: Request to Non-secure code (OS/Apps)

// Handled
#define REQUEST_NOT_HANDLED		0 //!< Valid request, but not handled 
#define REQUEST_HANDLED			1//!< Valid request, and handled

// Response
#define RESPONSE_SUCCESS		0
#define RESPONSE_FAILED			1

// Commands
#define SECDOMAIN_CMD_START					    (0)
//// Commands to SecOS  (1 - 63)
#define CMD_SECOS_EXIT					        (SECDOMAIN_CMD_START + 1)
#define CMD_SECOS_SECPROCESS_CREATE			    (SECDOMAIN_CMD_START + 2)
#define CMD_SECOS_SECPROCESS_RUN			    (SECDOMAIN_CMD_START + 3)
#define CMD_SECOS_SECPROCESS_FORCE_EXIT			(SECDOMAIN_CMD_START + 4)
#define CMD_SECOS_ALLOC_SHAREDMEM		        (SECDOMAIN_CMD_START + 5)
#define CMD_SECOS_DEALLOC_SHAREDMEM		        (SECDOMAIN_CMD_START + 6)
#define CMD_SECOS_SP_INFO_GET_IDS_NAMES		    (SECDOMAIN_CMD_START + 7)
//// Commands to SecProcesses  (64 - 127)
#define SECPROCESS_CMD_START                    (64)
#define CMD_SECPROCESS_SOFT_EXIT                (SECPROCESS_CMD_START)
#define CMD_SECPROCESS_NOTIFY					(SECPROCESS_CMD_START + 1)
#define SECDOMAIN_CMD_INVALID				    (128)
//// Commands to non-secure code (128 - 255)
#define NS_CMD_START					        (SECDOMAIN_CMD_INVALID)
#define CMD_NS_ALLOC_SHAREDMEM			        (NS_CMD_START + 1)
#define CMD_NS_DEALLOC_SHAREDMEM		        (NS_CMD_START + 2)
#define CMD_NOTIFY_NS					        (NS_CMD_START + 3)
#define NS_CMD_INVALID					        (256)

// Reserved Dest IDs
#define DEST_SECOS                              (0) // (O reserved for SecOS/OS, depending on the direction)
#define DEST_OS                                 (0) // (O reserved for SecOS/OS, depending on the direction)

#define SOS_COMM_OS_ACTUAL_SIZE		            (2 * (COMM_OS_CHANBUF_SIZE + COMM_OS_CHANCTL_SIZE))

#ifndef __ASSEMBLY__
typedef uint32_t msg_id_t;

//! Info associated with notifications sent from SecOS to untrusted OS/Apps
struct comm_os_app_siginfo_data
{
    spid_t  sp_id;
    ulong_t info1;
    ulong_t info2;
};

//! Ring buffer protocol for SecOS-OS channel
struct comm_os_header{
	msg_id_t	req_id;
	uint32_t	handled:2, //!< Is the request handled. must be 2-bits, because using 0 here may cause errors (empty msg have 0 value)
				direction:2, //!< Is the request handled. must be 2-bits, because using 0 here may cause errors (empty msg have 0 value)
				cmd:8,
				req_response:1,
				response:5,
				__reserved1:14;
	
	uint32_t	data_size:16,
				__reserved2:16;
				
	uint32_t	src_id; //!< the source SecApp ID or App ID (O reserved for SecOS/OS, depending on the direction)
	uint32_t	dest_id; //!< The destination SecApp ID or App ID (O reserved for SecOS/OS, depending on the direction)
	msg_id_t	ack_req_id; //!< The request id which we response to
	uint8_t		__reserved3[COMM_HEADER_SIZE - 24];
};

#define COMM_OS_MSG_HEADER_SIZE		(COMM_HEADER_SIZE)
struct comm_os_msg
{
	struct comm_os_header header;

	uint8_t	req_data[COMM_OS_MSG_PAYLOAD_SIZE];
};

struct comm_os_msg_free_sharedmem
{
	offset_t 	sharedmem_offset;
	size_t		sharedmem_sz;
};

// The shared information of ring buffer between SecOS and OS
struct comm_os_ctl
{
	uint32_t 	p_slot; //!< producer slot: the next msg slot to produce
	uint32_t	c_slot;
};

#define CHAN_OS_CTL(chan)	(volatile struct comm_os_ctl*)((chan)->comm_ctl_vaddr)
#define CHAN_OS_BUF(chan)	(volatile struct comm_os_msg*)((chan)->comm_buf_vaddr)

#endif // __ASSEMBLY__
#endif // VK_OS_CHANNEL_H