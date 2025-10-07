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

//! @file
//! This file contains public definitions shared by both the SecOS and the OS
//! modules (in which the OS modules acts as a helper/agent in communicating
//! with SecOS
#ifndef VK_OS_MODULE_H
#define VK_OS_MODULE_H


#ifndef __ASSEMBLY__

/// @brief Configure SecOS to be compatible with a specific OS.
typedef enum 
{
	SOS_COMPATIBILITY_INVALID = 0,
	SOS_COMPATIBILITY_LINUX = 1,
	SOS_COMPATIBILITY_WINDOWS = 2,
	SOS_COMPATIBILITY_MACOS = 3
} SOS_COMPATIBILITY_CONF;

struct vk_ret_status
{
	VKSTATUS status; 

	uint32_t intr[8];
};

#define OS_INTRRUPTS_NEED_IMMEDIATE_FORWARDING_NUM	(8)

/// @brief SecOS boot parameters.
/// [NOTE] SecOS does not have security-sensitive parameters at current.
struct sos_boot_params
{
	/********* SecOS Communication channel *********/
	/// @brief  the vector used by SecOS communication channel (both SecOS-OS channel and SecOS-SecBase channel, as they 
	/// share the same interrupt vector in SecOS)
	intr_t	 				vk_comm_intr;

	/// @brief  Red OS's dedicated CPU for SecOS-OS communications. SecOS must never run on this CPU.
	ulong_t					os_comm_cpu_lapic_id;

	SOS_COMPATIBILITY_CONF	sos_compat_conf;

	/// @brief If SecOS receives these red OS's interrupts, it needs to forward them to the red OS immediately.
	intr_t					os_intrs_immediate[OS_INTRRUPTS_NEED_IMMEDIATE_FORWARDING_NUM];
	uint32_t 				os_intrs_immediate_num;

	// Status
	struct vk_ret_status	status;
};

#endif // __ASSEMBLY__

#endif // VK_OS_MODULE_H