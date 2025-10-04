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

/*
 * This file contains balloon driver routines
 *
 * Copyright (c) 2009-2017  Red Hat, Inc.
 *
 * Author(s):
 *  Vadim Rozenfeld <vrozenfe@redhat.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met :
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and / or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of their contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// #include <ntddk.h> // If <ntifs.h> is used, then using <ntddk.h> causes conflicts of type definitions.
#include <ntifs.h>

#include "win_mm.h"

static void _mm_sdom_print_entry(int idx);

#define S_MEM_INFO_ENTRIES  (4096)
struct s_mem_info_entry
{
    vaddr_t kvaddr;
    size_t size;
    uvaddr_t uvaddr; // user-space vaddr

    PMDL mdl; // OS private data for s_mem
};

/// @brief Use spinlock to ensure that reads and writes to <s_mem_info> must be serialized
static lock_t s_mem_info_lock;
static struct s_mem_info_entry s_mem_info[S_MEM_INFO_ENTRIES] = {0};

static void _clean_entry(struct s_mem_info_entry* entry)
{
    entry->kvaddr = INVALID_VADDR;
    entry->size = 0;
    entry->uvaddr = INVALID_VADDR;
    entry->mdl = NULL;
}

static void _slot_map_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, S_MEM_INFO_ENTRIES, S_MEM_INFO_ENTRIES, 0, 1)
    {
        _clean_entry(&s_mem_info[i]);
    }
}

static int _find_slot_by_uvaddr(uvaddr_t uvaddr)
{
    uint32_t i = 0;

    FOREACH_S(i, S_MEM_INFO_ENTRIES, S_MEM_INFO_ENTRIES, 0, 1)
    {
        struct s_mem_info_entry* cur_addr = &s_mem_info[i];
        if(cur_addr->uvaddr == uvaddr)
            return i;
    }
        
    return -1;
}

static int _find_slot_by_kvaddr(vaddr_t kvaddr)
{
    uint32_t i = 0;

    FOREACH_S(i, S_MEM_INFO_ENTRIES, S_MEM_INFO_ENTRIES, 0, 1)
    {
        struct s_mem_info_entry* cur_addr = &s_mem_info[i];
        if(cur_addr->kvaddr == kvaddr)
            return i;
    }
        
    return -1;
}




/********* Public functions *********/
void sa_s_mem_init(void)
{
    os->spinlock_init(&s_mem_info_lock);

    os->spinlock_lock(&s_mem_info_lock);
    _slot_map_clean();
    os->spinlock_unlock(&s_mem_info_lock);
}

void sa_s_mem_fini(void)
{
    os->spinlock_lock(&s_mem_info_lock);
    _slot_map_clean();
    os->spinlock_unlock(&s_mem_info_lock);
}

OSSTATUS sa_s_mem_register(vaddr_t kvaddr, size_t size, uvaddr_t uvaddr, PMDL mdl)
{
    OSSTATUS status = OS_SUCCESS;
    int idx = 0;

    os->spinlock_lock(&s_mem_info_lock);

    // Check: The <kvaddr> must not have been registered before
    idx = _find_slot_by_kvaddr(kvaddr);
    if(idx != -1)
    {
        DRV_ERR("Register s_mem kvaddr:0x%lX error: double registration!\n", kvaddr);
        status = E_INTERNAL_ERROR;
        goto out;
    }

    idx = _find_slot_by_kvaddr(INVALID_VADDR);
    if(idx == -1)
    {
        DRV_ERR("Register s_mem kvaddr:0x%lX error: No space!\n", kvaddr);
        status = E_INTERNAL_ERROR;
        goto out;
    }

    s_mem_info[idx].kvaddr = kvaddr;
    s_mem_info[idx].size = size;
    s_mem_info[idx].uvaddr = uvaddr;
    s_mem_info[idx].mdl = mdl;

    // On success
    status = OS_SUCCESS;

out:
    os->spinlock_unlock(&s_mem_info_lock);
    return status;
}

OSSTATUS sa_s_mem_unregister(vaddr_t kvaddr)
{
    OSSTATUS status = OS_SUCCESS;
    int idx = 0;

    os->spinlock_lock(&s_mem_info_lock);

    idx = _find_slot_by_kvaddr(kvaddr);
    if(idx == -1)
    {
        status = E_INV_ARG;
        goto out;
    }

    _clean_entry(&s_mem_info[idx]);

    // On success
    status = OS_SUCCESS;

out:
    os->spinlock_unlock(&s_mem_info_lock);
    return status;
}

vaddr_t sa_s_mem_get_kvaddr(uvaddr_t uvaddr)
{
    vaddr_t result = INVALID_VADDR;
    int idx = 0;

    os->spinlock_lock(&s_mem_info_lock);

    idx = _find_slot_by_uvaddr(uvaddr);
    if(idx == -1)
    {
        result = INVALID_VADDR;
        goto out;
    }

    result = s_mem_info[idx].kvaddr;

out:
    os->spinlock_unlock(&s_mem_info_lock);
    return result;
}

size_t sa_s_mem_get_size(uvaddr_t uvaddr)
{
    size_t result = 0;
    int idx = 0;

    os->spinlock_lock(&s_mem_info_lock);

    idx = _find_slot_by_uvaddr(uvaddr);
    if(idx == -1)
    {
        result = 0;
        goto out;
    }

    result = s_mem_info[idx].size;

out:
    os->spinlock_unlock(&s_mem_info_lock);
    return result;
}

PMDL sa_s_mem_get_mdl(uvaddr_t uvaddr)
{
    PMDL result = NULL;
    int idx = 0;

    os->spinlock_lock(&s_mem_info_lock);

    idx = _find_slot_by_uvaddr(uvaddr);
    if(idx == -1)
    {
        result = NULL;
        goto out;
    }

    result = s_mem_info[idx].mdl;

out:
    os->spinlock_unlock(&s_mem_info_lock);
    return result;
}




/********* Private Functions *********/
static vaddr_t _win_mm_map_memory_to_kernel_space(PMDL mdl)
{
    vaddr_t vaddr = INVALID_VADDR;

    vaddr = (vaddr_t)MmMapLockedPagesSpecifyCache(
        mdl, 
        KernelMode, 
        MmCached, // Specify cache type, e.g., MmCached or MmNonCached
        NULL, 
        FALSE, 
        NormalPagePriority
    );

    return vaddr;
}

static uvaddr_t _win_mm_map_memory_to_user_space(PMDL mdl)
{
    uvaddr_t uvaddr = INVALID_VADDR;

    try
    {
        uvaddr = (uvaddr_t)MmMapLockedPagesSpecifyCache(
            mdl, 
            UserMode, 
            MmCached, // Specify cache type, e.g., MmCached or MmNonCached
            NULL, 
            FALSE, 
            NormalPagePriority
        );

        if (!uvaddr) 
        {
            return INVALID_VADDR;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) 
    {
        return INVALID_VADDR;
    }

    // On success
    return uvaddr;
}




/********* Public Functions *********/
void* win_mm_alloc_and_lock_pages_for_sdom(uint32_t num_pages, uvaddr_t* out_uvaddr)
{
    OSSTATUS status = OS_SUCCESS;
    PHYSICAL_ADDRESS low_paddr;
    PHYSICAL_ADDRESS high_paddr;
    PHYSICAL_ADDRESS skip_bytes;
    PMDL pPageMdl;
    uvaddr_t uvaddr = INVALID_VADDR;
    vaddr_t kvaddr = INVALID_VADDR;

    // Check: params must be valid
    if(!num_pages)
        return NULL;

    // Reserve memory in the NS domain
    low_paddr.QuadPart = 0;
    high_paddr.QuadPart = (ULONGLONG)-1;
    skip_bytes.QuadPart = 0;

    pPageMdl = MmAllocatePagesForMdlEx(low_paddr, high_paddr, skip_bytes,
        num_pages * PAGE_SIZE, MmCached, MM_DONT_ZERO_ALLOCATION);

    if (!pPageMdl)
    {
        DRV_ERR("Alloc memory for S domain: Failed to allocate %u 4K pages!\n", num_pages);
        goto alloc_err;
    }

    // Check: the allocation must be good
    if (MmGetMdlByteCount(pPageMdl) != (num_pages * PAGE_SIZE_4K))
    {
        DRV_ERR("Alloc memory for S domain: Not all requested memory was allocated (%d/%d)!\n", MmGetMdlByteCount(pPageMdl), num_pages * PAGE_SIZE_4K);
        goto alloc_check_err;
    }

    // Map the memory to both the kernel space and the caller's user space
    kvaddr = _win_mm_map_memory_to_kernel_space(pPageMdl);
    if(!kvaddr)
    {
        DRV_ERR("Alloc memory for S domain: Failed to reference the memory in kernel space!\n");
        goto ref_kernel_mode_err;
    }

    uvaddr = _win_mm_map_memory_to_user_space(pPageMdl);
    if(!uvaddr)
    {
        DRV_ERR("Alloc memory for S domain: Failed to reference the memory in user space!\n");
        goto ref_user_mode_err;
    }

    // Register the memory info
    status = sa_s_mem_register(kvaddr, num_pages * PAGE_SIZE_4K, uvaddr, pPageMdl);
    if(SA_FAIL(status))
    {
        DRV_ERR("Alloc memory for S domain: Registration error! status:%lu\n", status);
        goto register_err;
    }

    // On success
    if(out_uvaddr)
        *out_uvaddr = uvaddr;
    return (void*)kvaddr;


register_err:
    if(uvaddr)
        MmUnmapLockedPages((PVOID)uvaddr, pPageMdl);
ref_user_mode_err:
    if(kvaddr)
        MmUnmapLockedPages((PVOID)kvaddr, pPageMdl);
ref_kernel_mode_err:
alloc_check_err:
    if(pPageMdl)
    {
        MmFreePagesFromMdl(pPageMdl);
        ExFreePool(pPageMdl);
    }
alloc_err:
    return NULL;
}

void win_mm_free_and_unlock_pages_for_sdom_by_kvaddr(void* ptr)
{
    int idx = 0;

    // Check: params must be valid
    if(!ptr)
        return;

    os->spinlock_lock(&s_mem_info_lock);

    idx = _find_slot_by_kvaddr((vaddr_t)ptr);
    if(idx == -1)
        goto out;

    if(!s_mem_info[idx].mdl)
        goto out;

    // Free the memory
    if(s_mem_info[idx].kvaddr)
        MmUnmapLockedPages((PVOID)s_mem_info[idx].kvaddr, s_mem_info[idx].mdl);
    if(s_mem_info[idx].uvaddr)
        MmUnmapLockedPages((PVOID)s_mem_info[idx].uvaddr, s_mem_info[idx].mdl);

    MmFreePagesFromMdl(s_mem_info[idx].mdl);
    ExFreePool(s_mem_info[idx].mdl);

    _clean_entry(&s_mem_info[idx]);

out:
    os->spinlock_unlock(&s_mem_info_lock);
    return;
}

void win_mm_free_and_unlock_pages_for_sdom_by_uvaddr(uvaddr_t uvaddr)
{
    vaddr_t kvaddr = INVALID_VADDR;

    // Check: params must be valid
    if(!uvaddr)
        return;

    kvaddr = sa_s_mem_get_kvaddr(uvaddr);
    win_mm_free_and_unlock_pages_for_sdom_by_kvaddr((void*)kvaddr);
}




/********* Debug Functions *********/
DECL_UNUSED static void _mm_sdom_print_entry(int idx)
{
    struct s_mem_info_entry* entry = &s_mem_info[idx];
    DRV_ERR("Dump MM SDom entry [%d]: kvaddr:0x%llX, size:0x%llX, uvaddr:0x%llX, mdl:0x%p \n", idx, (vaddr_t)entry->kvaddr, (uint64_t)entry->size, (vaddr_t)entry->uvaddr, entry->mdl);
}