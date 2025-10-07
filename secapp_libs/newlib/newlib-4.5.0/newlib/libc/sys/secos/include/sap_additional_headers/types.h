// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_TYPES_H
#define SECAPP_TYPES_H

#include <sys/types.h>

#define UL(x)		(unsigned long)(x)
#define ULL(x)		(unsigned long long)(x)

#define BITS_PER_BYTE_SHIFT		3
#define BITS_PER_BYTE	        (1 << BITS_PER_BYTE_SHIFT)
#define BITS_TO_BYTES(x)		((x) >> BITS_PER_BYTE_SHIFT)
#define BYTES_TO_BITS(x)		((x) << BITS_PER_BYTE_SHIFT)

// #define BIT_SET			1
// #define BIT_CLEAR		0

// #define CHAR_BIT 8

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

typedef unsigned long addr_t;
typedef unsigned long vaddr_t;
typedef unsigned long paddr_t;
typedef unsigned long offset_t; // [NOTE] offset_t must be of the same length with vaddr_t, because
							// it is used in address offset calculation
typedef unsigned long spaddr_t;
typedef unsigned long irq_t;
typedef u64 vaddr64_t;
typedef unsigned long ulong;
typedef unsigned long ulong_t;

#define INVALID_HANDLE		0
#define INVALID_SPADDR		0
#define INVALID_PADDR		0
#define INVALID_VADDR		0

#define INVALID_INTR	(0xFF)

// Return the low 32 bits
#define LOW32(v)    ((uint32_t)v)

// Return the high 32 bits
#define HIGH32(v)   ((uint32_t)(((uint64_t)v) >> 32))

typedef union _QWORD
{
	struct
	{
		uint32_t low;
		uint32_t high;
	}__attribute__((packed));
	
	uint64_t val;
} QWORD;

typedef uint32_t 	shandle_t;  // Security handles. We do not use the name handle_t because it may have already defined in red OSes.
typedef uint32_t 	sig_no_t; 	// Signal Number
typedef uint32_t 	cpu_idx; 	// cpu idx
typedef uint8_t 	intr_t;

#ifndef bool
	typedef u8 bool;
	#define true 1
	#define false 0
#endif


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define mfence() __asm__ __volatile__("mfence": : :"memory")
#define barrier() __asm__ __volatile__("": : :"memory")


#define PACKED 	__attribute__((packed))

#endif //__ASSEMBLY__
#endif // SECAPP_TYPES_H