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

#ifndef UXMHF64_COMPILERS_H
#define UXMHF64_COMPILERS_H

#ifndef __ASSEMBLY__

#ifdef _MSC_VER

    #define PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
    #define PACKED_STRUCT_END __pragma(pack(pop))
    #define PACKED_STRUCT

#elif defined(__GNUC__)
    #define PACKED_STRUCT_BEGIN
    #define PACKED_STRUCT_END
    #define PACKED_STRUCT __attribute__((packed))
#endif // _MSC_VER


#endif // __ASSEMBLY__
#endif //UXMHF64_COMPILERS_H