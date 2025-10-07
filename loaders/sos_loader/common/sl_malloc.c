// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

