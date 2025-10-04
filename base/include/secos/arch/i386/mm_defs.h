/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#ifdef __i386__

#ifndef SOS_X86_I386_MM_DEFS_H
#define SOS_X86_I386_MM_DEFS_H

#define ADDR_MAX	                    (0xFFFFFFFF)

#define SOS_KERN_BASE 					(0xC0000000)         //! vaddr. of SecOS kernel addr. space
#define SOS_HIGHDEV_BASE 				(0xFE000000)         //! Other devices are at high addresses, vaddr = paddr

#endif // SOS_X86_I386_MM_DEFS_H
#endif // __i386__