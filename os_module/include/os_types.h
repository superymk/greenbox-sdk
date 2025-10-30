// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef OS_TYPES_H
#define OS_TYPES_H

#ifdef WINNT
#pragma warning(disable: 4296) // Disable warning C4296: "conditional expression is constant">=" expression is always true
#endif // WINNT

#ifdef __ASSEMBLY__
#define _AC(X,Y)	X
#define _AT(T,X)	X
#else
#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#define _AT(T,X)	((T)(X))
#endif

#define _UL(x)		(_AC(x, UL))
#define _ULL(x)		(_AC(x, ULL))

#define UL(x)		(_UL(x))
#define ULL(x)		(_ULL(x))

#ifndef __ASSEMBLY__

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
// typedef unsigned long vaddr_t;
// typedef unsigned long paddr_t;

typedef ulong_t offset_t; // [NOTE] offset_t must be of the same length with vaddr_t, because
							// it is used in address offset calculation
typedef ulong_t vaddr_t;
typedef ulong_t paddr_t;
typedef ulong_t uvaddr_t;
// typedef ulong_t ulong_t;

#define INVALID_PADDR	(0)
#define INVALID_VADDR	(0)
#define INVALID_INTR	(0xFF)
#define INVALID_IRQ		(0xFFFFFFFF)

#define INTR_VECTOR_MAX		(0xFF)

typedef u64 uint64_t;
typedef u32 uint32_t;
typedef u16 uint16_t;
typedef u8 uint8_t;

typedef u32 sp_id_t;
typedef uint32_t lapic_id_t;

/// @brief both i386 and amd64 uses 32-bit int for <lock_t>.
/// [NOTE] <lock_t> must have the same definition with it defined in SecOS.
typedef int lock_t;

#ifndef _PROCESS_ID_T
#define _PROCESS_ID_T
	typedef ulong_t process_id;
#endif // _PROCESS_ID_T

#define INVALID_UNTRUSTED_PROC_ID     	(0)
#define INVALID_LAPIC_ID				(0xFFFFFFFF)	// 0 is a valid LAPIC ID

#define UNUSED(x) (void)(x)

#define UINT32_TO_64(high, low)          ((uint64_t)(((uint64_t)high << 32ULL) | low))

#define VALUE_IN_RANGE(v, x, y)                 ((v) >= (x) && (v) < (y))

#define C_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


#endif //__ASSEMBLY__
#endif // OS_TYPES_H
