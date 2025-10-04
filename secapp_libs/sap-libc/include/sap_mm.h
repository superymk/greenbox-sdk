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

#ifndef _PAL_MM_H
#define _PAL_MM_H



#ifndef __ASSEMBLY__
struct sap_user_params;

typedef enum {
    PAL_MEM_DMA = 0x1,	// From DMA Pool
  
} PAL_MEM_TYPE;

extern spaddr_t sap_mm_vaddr_to_spaddr(vaddr_t vaddr);
extern paddr_t sap_mm_vaddr_to_paddr(vaddr_t vaddr);
extern int pal_mm_create_pool(SP_UTASK* sp_params);
extern void* pal_mm_alloc_align(PAL_MEM_TYPE mem_type, u32 alignment, u32 size);
extern void* pal_mm_alloc(PAL_MEM_TYPE mem_type, u32 size);
extern int pal_mm_dealloc(PAL_MEM_TYPE mem_type, void* vaddr);
extern void pal_mm_destroy_pool(void);

#endif // __ASSEMBLY__
#endif // _PAL_MM_H