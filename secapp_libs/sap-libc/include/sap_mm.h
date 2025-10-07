// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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