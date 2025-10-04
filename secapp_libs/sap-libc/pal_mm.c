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

#include <palddk.h>
//#include "tlsf.h"

/************ Data **********/
/* FIXME: for quick implementation, DMA pool and Ordinary pool is put in PAL's data memory, so that both VK and PAL can access them */
// DMA & Mem pool 
static tlsf_t _dma_pool_tlsf = NULL;

static SP_UTASK* sap_params = NULL;


static void pal_mm_create_dmapool(void)
{
	if(sap_params->dma_pool && sap_params->dma_pool_size)
		_dma_pool_tlsf = tlsf_create_with_pool((void *)sap_params->dma_pool, sap_params->dma_pool_size);
}

paddr_t sap_mm_vaddr_to_paddr(vaddr_t vaddr) 
{
	return sos_sys_mm_vaddr_to_paddr(vaddr);
}

static void pal_mm_destroy_dmapool(void) {
	/* free tlsf data structure */
	if (_dma_pool_tlsf) {
		tlsf_destroy(_dma_pool_tlsf);
		_dma_pool_tlsf = NULL;
	}
}




/************ Public Functions **********/
int pal_mm_create_pool(SP_UTASK* sp_params) 
{
	sap_params = sp_params;

	pal_mm_create_dmapool();
	return 0;
}

void* pal_mm_alloc_align(PAL_MEM_TYPE mem_type, u32 alignment, u32 size) 
{
	/* FIXME: use tlsf library to memalign, to avoid passing page
	 * boundary, always set alignment to the length of alloc_size
	 * rounded to the near power of 2 */
	void* vaddr = NULL;

	if(mem_type == PAL_MEM_DMA)
	{
		if(alignment)
			vaddr = tlsf_memalign(_dma_pool_tlsf, alignment, size);
		else
			vaddr = tlsf_malloc(_dma_pool_tlsf, size);
	}

	if(vaddr)
		pal_mm_memset(vaddr, 0, size);
		
	return vaddr;
}

void* pal_mm_alloc(PAL_MEM_TYPE mem_type, u32 size) 
{
	return pal_mm_alloc_align(mem_type, 0, size);
}

int pal_mm_dealloc(PAL_MEM_TYPE mem_type, void* vaddr) {
	
	if (vaddr && (mem_type == PAL_MEM_DMA)) {
		tlsf_free(_dma_pool_tlsf, (void *)vaddr);
	}
	
	return 0;
}

void pal_mm_destroy_pool(void)
{
	pal_mm_destroy_dmapool();

	sap_params = NULL;
}

