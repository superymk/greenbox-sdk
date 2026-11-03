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

#ifndef PUBLIC_HEADERS_SECAPP_DEFS_H
#define PUBLIC_HEADERS_SECAPP_DEFS_H

#define SOS_MAX_SPs 				256  	//!< The maximum allowed number of PALs at the same time.
#define SP_USER_ID_START			(SOS_RESERVED_IDS + 1)
#define SP_MAX_ID					(SOS_MAX_SPs - 1) //!< The maximum ID allowed for SecApps


#ifndef __ASSEMBLY__

/// @brief Red application's process ID
typedef uint32_t 	app_pid_t;

/// @brief The ID of the SecProcess.
/// [NOTE] <spid_t> must be 32-bit long. This is the assumption of driver IDs in IOSM-USB2 and IOSM-FB.
typedef uint32_t 	spid_t;

#endif // __ASSEMBLY__
#endif // PUBLIC_HEADERS_SECAPP_DEFS_H