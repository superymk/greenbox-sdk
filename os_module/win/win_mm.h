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

#ifndef SECAGENT_WIN_MEMORY_H
#define SECAGENT_WIN_MEMORY_H

#include "win_cpu.h"
#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern void* win_mm_malloc_zeroed(size_t size);
extern void win_mm_free(void* ptr);

extern vaddr_t win_mm_alloc_pages(uint32_t num_pages, uint32_t type);

extern paddr_t win_mm_vaddr_to_paddr(vaddr_t vaddr);

extern vaddr_t win_mm_get_physical_contiguous_pages(uint32_t num_pages, uint32_t type);
extern vaddr_t win_mm_alloc_physical_contiguous_pages_nocache(uint32_t num_pages, uint32_t type);
extern void win_mm_free_physical_contiguous_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);
extern void win_mm_free_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);

extern void* win_ioremap_nocache(paddr_t paddr, size_t size);

extern void win_iounmap(void* ptr, size_t size);

extern void win_memset(void* ptr, char v, size_t size);
extern void win_memcpy(void *__to, const void *__from, size_t __n);
extern OSSTATUS win_copy_to_user(void* usr_dest, const void* kern_src, size_t size);
extern OSSTATUS win_copy_from_user(void* kern_dest, const void* usr_src, size_t size);
extern void win_mm_fence(void);
extern void win_wbinvd(void);

extern OSSTATUS win_cache_flush_range(vaddr_t vaddr, size_t size);

extern OSSTATUS win_mm_check_current_process_page_permission(ulong_t uvaddr, size_t size, bool* out_allowed_read_write);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_MEMORY_H