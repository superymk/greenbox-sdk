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

#ifndef VK_COMMON_INTERFACES_H
#define VK_COMMON_INTERFACES_H

#define SP_EXEC_PARAMS_SECURE_SIZE		((uint32_t)(2 * PAGE_SIZE_4K))
#define SP_EXEC_PARAMS_NONSECURE_SIZE 	((uint32_t)(8 * PAGE_SIZE_4K))

#define SP_NAME_MAX_SIZE			((size_t)(64))

#define SECPROCESS_HASH_LEN_BYTES            (32)

#ifndef __ASSEMBLY__

/// @brief The type of the security sensitive process (SecProcess).
enum sp_type
{
	/// @brief By default, SP_SERVICE SecProcesses are not interruptible and do not use FPU.
	SP_SERVICE = 1,

	/// @brief By default, SP_APPLICATION SecProcesses are interruptible and use FPU.
	SP_APPLICATION = 2, 
};

/// @brief Whether the SecProcess can be scheduled or not
/// By default, SecOS considers a SecProcess as schedulable (SP_SCHEDULABLE).
enum sp_schedulable
{
	SP_SCHEDULABLE = 1,
	SP_NOT_SCHEDULABLE = 2,
};

/*typedef enum {
	TP_USBDEV = 0x1,
    TP_TDDEV = 0x2,
  
} TPDEV_TYPE;

typedef enum {
	BUS_PCI = 0x1,
  
} BUS_TYPE; */



/********* For CMD_SECOS_SP_INFO_GET_IDS_NAMES *********/
#define SP_INFO_GET_IDS_NAMES_MAX_RESULTS	(1024)

/// @brief SecProcess's ID and name returned on OS's query in <CMD_SECOS_SP_INFO_GET_IDS_NAMES>
struct sp_info_id_name
{
	spid_t  sp_id;
	char 	sp_name[SP_NAME_MAX_SIZE];
};

/// @brief Entire response message data on OS's query in <CMD_SECOS_SP_INFO_GET_IDS_NAMES>
struct sp_info_id_name_resp
{
	uint32_t num_info;
	struct sp_info_id_name info[SP_INFO_GET_IDS_NAMES_MAX_RESULTS];
};

#endif // __ASSEMBLY__
#endif // VK_COMMON_INTERFACES_H