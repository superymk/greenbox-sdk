// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <os/os_headers.h>

#define UL(x)		(unsigned long)(x)
#define ULL(x)		(unsigned long long)(x)

// Return the low 32 bits
#define LOW32(v)    ((uint32_t)v)

// Return the high 32 bits
#define HIGH32(v)   ((uint32_t)(((uint64_t)v) >> 32))

#define UINT32_TO_64(high, low)          ((uint64_t)(((uint64_t)high << 32ULL) | low))


#ifndef __ASSEMBLY__

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

typedef _ulong ulong_t;

#ifdef addr_t
#undef addr_t
typedef ulong_t addr_t;
#endif // addr_t

typedef ulong_t vaddr_t;
typedef ulong_t paddr_t;
typedef ulong_t spaddr_t;
typedef ulong_t irq_t;
typedef ulong_t offset_t; // [NOTE] offset_t must be of the same length with vaddr_t, because
							// it is used in address offset calculation
typedef ulong_t lock_t;	// [NOTE] lock_t must have the same definition with it defined in <vk_lock.h>

#define INVALID_PADDR	0
#define INVALID_VADDR	0

typedef union _QWORD
{
	PACK(struct
	{
		uint32_t low;
		uint32_t high;
	});
	
	uint64_t val;
} QWORD;

typedef uint32_t 	shandle_t; // Security handles. We do not use the name handle_t because it may have already defined in red OSes.
typedef uint32_t 	sig_no_t; 	// Signal Number
typedef uint32_t 	cpu_idx; 	// cpu idx
typedef uint8_t 	intr_t;
typedef uint16_t 	pio_t; // [TODO] Move it to a better place. The definition needs to be consistent with the one in <vk_syscalls.h>

typedef uint32_t app_pid_t; //! Application process ID
typedef uint32_t spid_t;
#define INVALID_SP_ID                   0


typedef uint64_t perf_counter_t;

#ifndef NULL
	#define NULL ((void*)0)
#endif

#ifndef __GNUG__
#ifndef bool
	typedef uint8_t bool;
	#define true 1
	#define false 0
#endif
#endif

#define TO_PTR(x) (void*)(x)

#ifdef __i386__
#define uint32_to_ulong(low, high) ((ulong_t)low)
#elif defined (__amd64__)
#define uint32_to_ulong(low, high) ((((ulong_t)high << 32ULL) | low))
#endif // __i386__ __amd64__


#define roundup2(x, y) (((x) + ((y) - 1)) & (~((__typeof(x))(y) - 1)))
#define rounddown2(x, y) ((x) & ~((__typeof(x))(y) - 1))

#define ALIGN(x, y) roundup2((x), (y))

#define IS_ALIGNED(x, y)	(((x) & ((y) - 1)) == 0)
#define PTR_ALIGN(x, y)		((__typeof(x))roundup2((unsigned long)(x), (y)))
#define ALIGN_DOWN(x, y)	((__typeof(x))rounddown2((unsigned long)(x), (y)))


#define mfence() __asm__ __volatile__("mfence": : :"memory")
#define barrier() __asm__ __volatile__("": : :"memory")

#endif //__ASSEMBLY__
#endif // APP_TYPES_H