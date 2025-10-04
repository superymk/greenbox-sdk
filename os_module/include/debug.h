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

#ifndef SECAGENT_DEBUG_H
#define SECAGENT_DEBUG_H
#include <os_types.h>


#ifndef __ASSEMBLY__

extern void dbg_dumpmem(vaddr_t start_vaddr, size_t size);


#endif // __ASSEMBLY__
#endif // SECAGENT_DEBUG_H