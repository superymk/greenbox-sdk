// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdlib.h>
#include <string.h>
#include <sap_headers_libc.h>

/************ Data **********/
#define MAPPABLE_VADDR_END		(SOS_KERN_BASE)

static vaddr_t start_mappable_vaddr = INVALID_VADDR;
static vaddr_t next_mappable_vaddr = INVALID_VADDR;




/********* Functions - Remappable Memory Region *********/
static SAP_STATUS _mm_remappable_init(vaddr_t remappable_base)
{
	if (remappable_base >= MAPPABLE_VADDR_END)
	{
		// No vaddr space for remappable memory region. So the initialization function marks the <_remappable_tlsf> pool
		// as NULL, so the related malloc/free functions are unavailable.
		start_mappable_vaddr = INVALID_VADDR;
		return SUCCESS;
	}

	start_mappable_vaddr = remappable_base;
	next_mappable_vaddr = remappable_base;
	
	return SUCCESS;
}

static void _mm_remappable_fini(void)
{
	start_mappable_vaddr = INVALID_VADDR;
	next_mappable_vaddr = INVALID_VADDR;
}

vaddr_t mm_pick_vaddr(size_t mem_size)
{
	vaddr_t vaddr = next_mappable_vaddr;

	next_mappable_vaddr += mem_size;

	if(next_mappable_vaddr >= MAPPABLE_VADDR_END)
	{
		// Round Robin
		vaddr = next_mappable_vaddr = start_mappable_vaddr;
	}

	return vaddr;
}

void mm_return_vaddr(vaddr_t vaddr, size_t size)
{
	vaddr &= PAGE_MASK_4K;

	if(vaddr == (next_mappable_vaddr - size))
	{
		// We just return the latest picked vaddr.
		// So we can save some mappable vaddr space. (the last paddr mapping
		// request must be a short life one.
		next_mappable_vaddr -= size;
	}
}




/********* Functions - Secure Allocations *********/
/// Malloc, if successful, clear the data to be 0
void* smalloc(size_t size)
{
	void* ptr = NULL;

	ptr = malloc(size);
	if(ptr)
		memset(ptr, 0, size);
	
	mfence();
	return ptr;
}

void* smalloc_aligned(size_t alignment, size_t size)
{
	void* ptr = NULL;

	ptr = aligned_alloc(alignment, size);
	if(ptr)
		memset(ptr, 0, size);
	
	mfence();
	return ptr;
}

/// Clear the memory content and free
void sfree(void* ptr, size_t size)
{
	if(ptr)
	{
		memset(ptr, 0, size);
		free(ptr);
	}
	
	mfence();
}

/// @brief Allocate aligned pages from heap memory.
void* alloc_pages(uint32_t num_pages)
{
	return aligned_alloc(PAGE_SIZE_4K, num_pages << PAGE_SHIFT_4K);
}

/// @brief Frees allocated pages and return it to the heap.
/// 
/// @param mem_type is the type of heap memory where it allocated from.
/// @param vaddr is the pointer to the memory pages to be freed.
void free_pages(void* vaddr, uint32_t num_pages)
{
	return sfree(vaddr, num_pages*PAGE_SIZE_4K);
}




/********* Functions - Others *********/
SAP_STATUS sp_mm_init(vaddr_t remappable_base)
{
	SAP_STATUS status = SUCCESS;

	status = _mm_remappable_init(remappable_base);
	if(FAIL(status))
	{
		SAP_ERR("Remappable Virtual Memory Region Init Error! status:%lu", status);
		return EINTERNAL;
	}

	return SUCCESS;
}

void sp_mm_fini(void)
{
	_mm_remappable_fini();
}