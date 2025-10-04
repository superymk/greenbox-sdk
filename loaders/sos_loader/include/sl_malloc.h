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

#ifndef SECOS_LOADER_MALLOC_H
#define SECOS_LOADER_MALLOC_H
#include <sl_status.h>

#ifndef __ASSEMBLY__

extern SL_STATUS sl_mm_init_sharedmem_pool(vaddr_t shared_mem);

extern void* sl_mm_alloc_sharedmem_aligned(uint32_t alignment, uint32_t size);

extern void* sl_mm_alloc_sharedmem(uint32_t size);

extern void sl_mm_free_sharedmem(void* ptr);

extern offset_t sl_mm_sharedmem_to_offset(void* vaddr);

extern vaddr_t sl_mm_sharedmem_ptr(offset_t offset);

#endif // __ASSEMBLY__
#endif // SECOS_LOADER_MALLOC_H