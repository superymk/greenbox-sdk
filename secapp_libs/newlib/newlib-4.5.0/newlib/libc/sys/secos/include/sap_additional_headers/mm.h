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

#ifndef SECAPP_MM_H
#define SECAPP_MM_H


// #define KB(x) ((UL(x)) << 10)
// #define MB(x) ((UL(x)) << 20)
// #define GB(x) ((ULL(x)) << 30)


// #define PAGE_SHIFT_4K			12

// #define PAGE_SIZE_4K			(_AC(1, UL) << PAGE_SHIFT_4K)

// #define PAGE_MASK_4K			(~(PAGE_SIZE_4K-1))
// #define PAGE_FLAGS_MASK_4K		(~PAGE_MASK_4K)
// #define PAGE_OFFSET_4K			(PAGE_FLAGS_MASK_4K)



// Convert between frame number and address formats.  
#define PAGE4K_PFN_TO_PADDR(pfn)	    ((paddr_t)(pfn) << PAGE_SHIFT_4K)
#define PAGE4K_PADDR_TO_PFN(paddr)	    (((paddr & 0xfffff000) >> PAGE_SHIFT_4K))



#ifndef __ASSEMBLY__

#include <stddef.h>
#include <sap_additional_headers/types.h>
#include <sap_additional_headers/errno.h>

extern SAP_STATUS sp_mm_init(vaddr_t remappable_base);
extern void sp_mm_fini(void);


/// @brief Malloc, if successful, clear the data to be 0
/// 
/// @param size
extern void* smalloc(size_t size);

/// @brief Aligned Malloc, if successful, clear the data to be 0
/// 
/// @param alignment 
/// @param size
extern void* smalloc_aligned(size_t alignment, size_t size);

/// Clear the memory content and free
extern void sfree(void* ptr, size_t size);

/// @brief Allocate aligned pages from heap memory.
extern void* alloc_pages(uint32_t num_pages);

/// @brief Frees allocated pages and return it to the heap.
/// 
/// @param mem_type is the type of heap memory where it allocated from.
/// @param vaddr is the pointer to the memory pages to be freed.
extern void free_pages(void* vaddr, uint32_t num_pages);


extern vaddr_t mm_pick_vaddr(size_t mem_size);
extern void mm_return_vaddr(vaddr_t vaddr, size_t size);

#endif // __ASSEMBLY__
#endif // SDDK_MM_H