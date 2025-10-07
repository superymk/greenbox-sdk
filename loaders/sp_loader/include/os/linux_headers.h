// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SP_LOADER_OS_LINUX
#define SP_LOADER_OS_LINUX

#include <inttypes.h>
#include <stddef.h>

#ifndef __ASSEMBLY__


typedef unsigned long _ulong;
typedef _ulong ulong;

#ifndef __GNUG__
#ifndef bool
	typedef uint8_t bool;
	#define true 1
	#define false 0
#endif
#endif

#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#endif // __ASSEMBLY__
#endif // SP_LOADER_OS_LINUX