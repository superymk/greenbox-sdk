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

#ifndef SECBASE_ERRNO_H
#define SECBASE_ERRNO_H

#define MHV_SUCCESS		 						0
#define MHV_FAILED								999		// Represent status not changed

#define E_SECOS_HAS_REGISTERED_ON_CORE      101
#define E_SECOS_NOT_REGISTERED_ON_CORE      102
#define E_SECOS_HAS_BOOTED_ON_CORE			103
#define E_SECOS_NOT_BOOTED_ON_CORE			104
#define E_SECOS_UNSUPPORTED_CPU_MODE        105

#define E_MEDIATION_OUT_OF_RANGE			202
#define E_MEDIATION_INTERNAL				203
#define E_DOM_ISOLATION_VIOLATION			204 	// Domain isolation violated

#define MHV_E_MEM_SECTION_DUPLICATE_SPADDR  300

#define MHV_EPERM        	1  /* Operation not permitted */
//#define ENOENT       		2  /* No such file or directory */
//#define ESRCH        		3  /* No such process */
//#define EINTR        		4  /* Interrupted system call */
#define MHV_EIO      	 	5  /* I/O error */
//#define ENXIO        		6  /* No such device or address */
//#define E2BIG        		7  /* Argument list too long */
//#define ENOEXEC      		8  /* Exec format error */
//#define EBADF        		9  /* Bad file number */
//#define ECHILD      		10  /* No child processes */
//#define EAGAIN      		11  /* Try again */
#define MHV_ENOMEM      	12  /* Out of memory */
#define MHV_EACCES      	13  /* Permission denied */
#define MHV_EFAULT      	14  /* Bad address */
//#define ENOTBLK     		15  /* Block device required */
#define MHV_EBUSY       	16  /* Device or resource busy */
//#define EEXIST      		17  /* File exists */
//#define EXDEV       		18  /* Cross-device link */
#define MHV_ENODEV     		19  /* No such device */
//#define ENOTDIR     		20  /* Not a directory */
//#define EISDIR      		21  /* Is a directory */
#define MHV_EINVAL      	22  /* Invalid argument */
//#define ENFILE      		23  /* File table overflow */
//#define EMFILE      		24  /* Too many open files */
//#define ENOTTY      		25  /* Not a typewriter */
//#define ETXTBSY     		26  /* Text file busy */
//#define EFBIG       		27  /* File too large */
#define MHV_ENOSPC      	28  /* No space left on device */
//#define ESPIPE      		29  /* Illegal seek */
//#define EROFS       		30  /* Read-only file system */
//#define EMLINK      		31  /* Too many links */
//#define EPIPE       		32  /* Broken pipe */
//#define EDOM        		33  /* Math argument out of domain of func */
#define MHV_ETIMEOUT      	34  // Operation Timeout
#define MHV_EOVERFLOW       35  /* Integer overflow*/

#define MHV_E_INV_INSTRUCTION 97  // Invalid instruction

#define MHV_E_INV_HYPERCALL 98  // Invalid hypercall
#define MHV_EINTERNAL       99  // Other internal errors



#ifndef __ASSEMBLY__

#define MHV_FAIL(status)		(unlikely(status != MHV_SUCCESS))
#define MHV_PASS(status)		(likely(status == MHV_SUCCESS))

typedef ulong_t         MHV_STATUS;

#endif // __ASSEMBLY__
#endif // SECBASE_ERRNO_H