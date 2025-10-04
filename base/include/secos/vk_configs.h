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

#ifndef SECOS_COMMON_CONFIGS_H
#define SECOS_COMMON_CONFIGS_H

// SecOS Resources
#define SECOS_STACK_SIZE 			(5 * PAGE_SIZE_4K)
#define VK_MEM_SIZE 			 	MB(16)  // MB(4)
#define VK_SHAREMEM_SIZE			KB(256)     // MB(16)

#define SECOS_TIMER_HZ				(100)    // Timer interrupt frequency

// Communication Channels
#define COMM_MSG_SIZE_ORDER			6
//! The size of each comm_msg is 64 bytes
#define COMM_MSG_SIZE				(uint32_t)(1 << COMM_MSG_SIZE_ORDER)
#define COMM_HEADER_SIZE			32//! Bytes used for a message header (used in *-* communication,
							//! e.g. SecAgent-App comm, SecOS-OS (SecAgent) comm.

// SecOS-OS communication channel
#define COMM_OS_MSG_SIZE			((uint32_t)COMM_MSG_SIZE)
#define COMM_OS_MSG_PAYLOAD_SIZE	(COMM_OS_MSG_SIZE - COMM_HEADER_SIZE)
#define COMM_OS_CHAN_NUM_MSG_SLOTS	(COMM_OS_CHANBUF_PAGES << (PAGE_SHIFT_4K - COMM_MSG_SIZE_ORDER))



// The size of the ring buffer used by SecOS-OS communication channel
#define COMM_OS_CHANBUF_PAGES_ORDER		1
#define COMM_OS_CHANBUF_PAGES			(1 << COMM_OS_CHANBUF_PAGES_ORDER)
#define COMM_OS_CHANBUF_SIZE			(COMM_OS_CHANBUF_PAGES << PAGE_SHIFT_4K)
#define COMM_OS_CHANCTL_PAGES_ORDER		0
#define COMM_OS_CHANCTL_PAGES			(1 << COMM_OS_CHANCTL_PAGES_ORDER)
#define COMM_OS_CHANCTL_SIZE			(COMM_OS_CHANCTL_PAGES << PAGE_SHIFT_4K)

// SecApp configurations in SecOS
#define PAL_KERN_STACK_PAGES		2
#define PAL_KERN_STACK_SIZE			(PAL_KERN_STACK_PAGES << PAGE_SHIFT_4K)
#define PAL_MAX_DEVICES				16


#endif // SECOS_COMMON_CONFIGS_H