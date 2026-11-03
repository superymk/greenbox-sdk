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

#ifndef SECOS_COMMON_DEFS_H
#define SECOS_COMMON_DEFS_H

#define INVALID_SP_ID                   0

//! The invalid handler of the IOMMU PageTable
#define IOMMU_PT_INVALID	            0xFFFFFFFF


// Default ID (sp_id/app_id)
// The reserved ID for Sec* components in OS and SecOS
#define SECOS_ID					0
#define SECBASE_ID					1
#define OS_SYS_ID					2

#define SOS_RESERVED_IDS            4


#ifndef __ASSEMBLY__

typedef uint32_t cpu_idx; 	// cpu index
typedef uint8_t intr_t;

#endif // __ASSEMBLY__
#endif // SECOS_COMMON_DEFS_H

