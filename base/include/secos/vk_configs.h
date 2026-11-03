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