// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SP_LOADER_OS_WINDOWS
#define SP_LOADER_OS_WINDOWS

// [NOTE] We do not include <windows.h> here, or we may see re-definitions
#include <BaseTsd.h>

#ifndef __ASSEMBLY__

// #pragma warning(disable: 4127) // Disable warning C4127: conditional expression is constant
// #pragma warning(disable: 4201) // Disable warning C4201: nonstandard extension used: nameless struct/union

// Additional typedefs required by windows
typedef ULONG_PTR _ulong;
// [NOTE] We cannot define the type <ulong>, because it will be defined by cygwin-x64 (as a 32-bit value?)

#if defined(__CYGWIN__) && defined (__amd64__)
typedef unsigned long uint64_t;
#else
typedef unsigned long long uint64_t;
#endif

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;



#ifdef _MSC_VER
	#include <stdbool.h>
#elif defined (__MSYS2__)
	#ifndef __GNUG__
	#ifndef bool
		typedef uint8_t bool;
		#define true 1
		#define false 0
	#endif
	#endif
#endif // __MSYS2__

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#elif defined (__MSYS2__)
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif // _MSC_VER

#ifdef _MSC_VER // Check if using Visual C++
#define likely(x)       (x)
#define unlikely(x)     (x)
#elif defined (__MSYS2__)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif // _MSC_VER


#endif // __ASSEMBLY__
#endif // SP_LOADER_OS_WINDOWS