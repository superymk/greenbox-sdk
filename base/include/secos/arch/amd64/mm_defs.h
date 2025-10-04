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

#ifdef __amd64__

#ifndef SOS_X86_AMD64_MM_DEFS_H
#define SOS_X86_AMD64_MM_DEFS_H

#define ADDR_MAX                        (0xFFFFFFFFFFFFFFFF)

#define SOS_KERN_BASE 					(0xFFFFFFFFC0000000)         //! vaddr. of SecOS kernel addr. space
#define SOS_HIGHDEV_BASE 				(0xFFFFFFFFFE000000)         //! Other devices are at high addresses, vaddr = paddr

#endif // SOS_X86_AMD64_MM_DEFS_H
#endif // __amd64__