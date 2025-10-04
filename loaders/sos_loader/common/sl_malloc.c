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

#include <sl_headers.h>
//#include <tlsf.h>
#include <app_os_if.h>

/*static vaddr_t sharedmem_base_vaddr = INVALID_VADDR;

SL_STATUS sl_mm_init_sharedmem_pool(vaddr_t shared_mem)
{
	// Prevent double creation
	if(sharedmem_base_vaddr)
		return SL_EINVAL;

	sharedmem_base_vaddr = shared_mem;

	return SL_SUCCESS;
}

void* sl_mm_alloc_sharedmem_aligned(uint32_t alignment, uint32_t size)
{
	offset_t offset = 0;
	
	offset = secagent_alloc_smem(alignment, size);

	if(offset == (offset_t)(-1))
		return NULL;

	return (void*)sl_mm_sharedmem_ptr(offset);
}

void* sl_mm_alloc_sharedmem(uint32_t size)
{
	return sl_mm_alloc_sharedmem_aligned(0, size);
}

void sl_mm_free_sharedmem(void* ptr)
{
	offset_t offset = sl_mm_sharedmem_to_offset(ptr);

	secagent_dealloc_smem(offset);
}

offset_t sl_mm_sharedmem_to_offset(void* vaddr)
{
	return (vaddr_t)vaddr - sharedmem_base_vaddr;
}

vaddr_t sl_mm_sharedmem_ptr(offset_t offset)
{
	return (vaddr_t)(offset + sharedmem_base_vaddr);
}*/

