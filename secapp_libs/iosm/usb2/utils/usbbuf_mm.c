// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "../lib-iosm-usb-internal.h"
//#include "tlsf.h"


/* FIXME: for quick implementation, DMA pool and Ordinary pool is put in PAL's data memory, so that both VK and PAL can access them */
// DMA & Mem pool 
static tlsf_t usbbuf_pool = NULL;
static vaddr_t usbbuf_vaddr = INVALID_VADDR;


/************ Public Functions **********/
SAP_STATUS usbbuf_create_pool(vaddr_t in_usbbuf_vaddr, size_t usbbuf_size)
{
	if((in_usbbuf_vaddr == INVALID_VADDR) || (!usbbuf_size))
		return EINVAL;

	usbbuf_vaddr = in_usbbuf_vaddr;
	usbbuf_pool = tlsf_create_with_pool((void *)usbbuf_vaddr, usbbuf_size);
    if(!usbbuf_pool)
        return EINTERNAL;

    return SUCCESS;
}

void usbbuf_destroy_pool(void)
{
	if (usbbuf_pool) 
	{
		tlsf_destroy(usbbuf_pool);
		usbbuf_pool = NULL;
	}
}

void* usbbuf_alloc_align(USBBUF_MEM_TYPE mem_type, uint32_t alignment, size_t size) 
{
	/* FIXME: use tlsf library to memalign, to avoid passing page
	 * boundary, always set alignment to the length of alloc_size
	 * rounded to the near power of 2 */
	void* vaddr = NULL;

	if(mem_type == USBBUF_MEM_DMA)
	{
		if(alignment)
			vaddr = tlsf_memalign(usbbuf_pool, alignment, size);
		else
			vaddr = tlsf_malloc(usbbuf_pool, size);
	}

	if(vaddr)
		pal_mm_memset(vaddr, 0, size);
		
	return vaddr;
}

void* usbbuf_alloc(USBBUF_MEM_TYPE mem_type, size_t size) 
{
	return usbbuf_alloc_align(mem_type, 0, size);
}

void usbbuf_dealloc(USBBUF_MEM_TYPE mem_type, void* vaddr) 
{
	if (vaddr && (mem_type == USBBUF_MEM_DMA))
		tlsf_free(usbbuf_pool, (void *)vaddr);
}

paddr_t usbbuf_get_base_paddr(void)
{
	return sap_mm_vaddr_to_paddr(usbbuf_vaddr);
}