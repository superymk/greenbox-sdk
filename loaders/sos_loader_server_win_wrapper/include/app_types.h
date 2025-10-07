// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_TYPES_H
#define APP_TYPES_H

#ifndef __ASSEMBLY__

#if defined(__CYGWIN__) && defined (__amd64__)
typedef unsigned long uint64_t;
#else
typedef unsigned long long uint64_t;
#endif

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#endif // __ASSEMBLY__
#endif // APP_TYPES_H