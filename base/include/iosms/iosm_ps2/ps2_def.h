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

#ifndef IOSM_PS2_DEF_H
#define IOSM_PS2_DEF_H

// Ports
#define PS2_CMD 	0x64
#define PS2_STATUS 	0x64
#define PS2_DATA 	0x60

#define PS2_CMD_PORT_WIDTH 	    (4)
#define PS2_DATA_PORT_WIDTH 	(4)


#ifndef __ASSEMBLY__


#endif // __ASSEMBLY__
#endif // IOSM_PS2_DEF_H
