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

#ifdef __amd64__

#ifndef SOS_X86_AMD64_MM_DEFS_H
#define SOS_X86_AMD64_MM_DEFS_H

#define ADDR_MAX                        (0xFFFFFFFFFFFFFFFF)

#define SOS_KERN_BASE 					(0xFFFFFFFFC0000000)         //! vaddr. of SecOS kernel addr. space
#define SOS_HIGHDEV_BASE 				(0xFFFFFFFFFE000000)         //! Other devices are at high addresses, vaddr = paddr

#endif // SOS_X86_AMD64_MM_DEFS_H
#endif // __amd64__