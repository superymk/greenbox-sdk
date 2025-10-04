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

#ifndef SECAGENT_WIN_MEMORY_SDOM_H
#define SECAGENT_WIN_MEMORY_SDOM_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern void sa_s_mem_init(void);

extern void sa_s_mem_fini(void);

extern void* win_mm_alloc_and_lock_pages_for_sdom(uint32_t num_pages, uvaddr_t* out_uvaddr);
extern void win_mm_free_and_unlock_pages_for_sdom_by_kvaddr(void* ptr);
extern void win_mm_free_and_unlock_pages_for_sdom_by_uvaddr(uvaddr_t uvaddr);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_MEMORY_SDOM_H