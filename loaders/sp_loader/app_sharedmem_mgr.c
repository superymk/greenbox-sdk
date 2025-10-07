// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <app_sharedmem_mgr.h>
#include "tlsf.h"

static vaddr_t smem_base = INVALID_VADDR;

static tlsf_t smem_pool = NULL;

int smem_mgr_init(SP_USTRUCT* params)
{
	struct sp_addrs_info* sp_addrs = NULL;
	size_t mem_size = 0;

	if(!params)
		return 1;

	sp_addrs = &params->sp_addrs;
	mem_size = sp_addrs->sp_sharemem_end - sp_addrs->sp_sharemem_start;
	smem_base = sp_addrs->sp_sharemem_start;

	// Skip initialization of SecProcess's shared memory, if it is not allocated
	if(params->sp_addrs.sp_sharemem_start == INVALID_VADDR)
	{
		APPCOMM_INFO("No allocator for SecProcess's shared memory is created\n");
		return 0;
	}

	smem_pool = tlsf_create_with_pool((void*)sp_addrs->sp_sharemem_start, mem_size);

	// On success
	return 0;
}

void smem_mgr_fini(SP_USTRUCT* params)
{
	if(!params)
		return;

	if(params->sp_addrs.sp_sharemem_start == INVALID_VADDR)
	{
		APPCOMM_INFO("No allocator for SecProcess's shared memory needs to be finalized\n");
		return;
	}
	
	if (smem_pool)
	{
		tlsf_destroy(smem_pool);
		smem_pool = NULL;
	}
}

inline vaddr_t smem_get_base(void)
{
	return smem_base;
}

smem_offset_t smem_allocate(size_t size)
{
	void* ptr = NULL;

	ptr = tlsf_malloc(smem_pool, size);
	if(ptr)
	{
		app_mm_memset(ptr, 0, size);		
		return SMEM_VADDR_TO_OFFSET(ptr);
	}

	return INVALID_SMEM_OFFSET;
}

void smem_free(smem_offset_t offset)
{
	void* ptr = (void*)SMEM_OFFSET_TO_VADDR(offset);
	if(offset != INVALID_SMEM_OFFSET)
		tlsf_free(smem_pool, ptr);
}

