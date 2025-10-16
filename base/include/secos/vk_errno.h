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

#ifndef VK_ERRNO_H
#define VK_ERRNO_H

#define SUCCESS		 			0
#define FAILED					999		//! Represent status not changed

#define E_ASSERT				101
#define E_KERNEL_HALT			102
#define E_TRAP					103
#define E_SECURITY				104
#define EINVDEV		 			105	    //! Not a valid device for that type of drivers
#define ENOTHANDLED				107		//! Not handled
#define E_NO_EXECUTION			108		//! Hit the instruction which should not execute
#define E_OBJ_NOTEXIST			109		//! Object does not exist
#define E_OBJ_NOTOWNED			110		//! Object not owned by the requester
#define E_OBJ_NOT_USER			111		//! the requester is not the user of the object
#define E_OBJ_NOT_MEMOBJ		112		//! the target object is not a memory object
#define E_MEMOBJ_DUP_MAPPING    113     /// The memory object has been mapped to some virtual address
#define E_SP_INVALID			120		//! Invalid SecProcess
#define SECOS_E_EXCEPTION_PF	130
#define SECOS_E_EXCEPTION_OTHER	135

#define ECPU		 			201 	//! CPU Error
#define EAPIC		 			202 	//! APIC Error
#define EIRQ		 			203 	//! IRQ handling error
#define ETIMER		 			204 	//! Timer subsystem error

/// @brief uTPM is not available
#define E_UTPM_NOT_AVAILABLE    250

#define SECOS_INFO_PREEMPTED		801
#define SECOS_INFO_RETURN_FROM_PREEMPTION	    803     //! SecOS resumes after red OS preempts its execution
                                            //! SecAgent needs to know this status so it can correctly send IPIs to SecOS
#define SECOS_INFO_EXITED			804

#define E_NOT_IMPLEMENTED		900		//! Not implemented


#define EPERM        	1  /* Operation not permitted */
//#define ENOENT       		2  /* No such file or directory */
//#define ESRCH        		3  /* No such process */
//#define EINTR        		4  /* Interrupted system call */
#define EIO      	 	5  /* I/O error */
//#define ENXIO        		6  /* No such device or address */
//#define E2BIG        		7  /* Argument list too long */
//#define ENOEXEC      		8  /* Exec format error */
//#define EBADF        		9  /* Bad file number */
//#define ECHILD      		10  /* No child processes */
// #define EAGAIN      		11  /* Try again */
#define ENOMEM      	12  /* Out of memory */
#define EACCES      	13  /* Permission denied */
#define EFAULT      	14  /* Bad address */
//#define ENOTBLK     		15  /* Block device required */
#define EBUSY       	16  /* Device or resource busy */
//#define EEXIST      		17  /* File exists */
//#define EXDEV       		18  /* Cross-device link */
#define ENODEV     		19  /* No such device */
//#define ENOTDIR     		20  /* Not a directory */
//#define EISDIR      		21  /* Is a directory */
#define EINVAL      	22  /* Invalid argument */
//#define ENFILE      		23  /* File table overflow */
//#define EMFILE      		24  /* Too many open files */
//#define ENOTTY      		25  /* Not a typewriter */
//#define ETXTBSY     		26  /* Text file busy */
//#define EFBIG       		27  /* File too large */
#define ENOSPC      	28  /* No space left on device */
//#define ESPIPE      		29  /* Illegal seek */
//#define EROFS       		30  /* Read-only file system */
//#define EMLINK      		31  /* Too many links */
//#define EPIPE       		32  /* Broken pipe */
//#define EDOM        		33  /* Math argument out of domain of func */
#define ETIMEOUT      	34  // Operation Timeout
#define S_EOVERFLOW       35  /* Integer overflow*/


#define EINTERNAL       99  // Other internal errors

#ifndef __ASSEMBLY__

#define FAIL(status)		(unlikely(status != SUCCESS))
#define PASS(status)		(likely(status == SUCCESS))
#define CHKFAIL(status)		(unlikely(status == EPERM))

typedef ulong_t VKSTATUS;


#endif // __ASSEMBLY__
#endif // VK_ERRNO_H