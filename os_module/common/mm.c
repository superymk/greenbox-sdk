// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

// OSSTATUS do_CMD_APP_CACHE_FLUSH_RANGE(struct tz_cmd_packet *cmd_packet)
// {
//     OSSTATUS status = OS_SUCCESS;
//     vaddr_t start_vaddr = (vaddr_t)cmd_packet->arg1;    // arg1 - the start virtual address of the memory to be cache flushed
//     size_t mem_sz = (size_t)cmd_packet->arg2;           // arg2 - the size of the memory
//     bool flush_all_data_cache_on_failure = (bool)cmd_packet->arg3;           // arg3 - Flush all data cache if failed to 
//                                             // flush the given cache range.

//     status = os->cache_flush_range(start_vaddr, mem_sz);
//     if(SA_FAIL(status))
//     {
//         if(flush_all_data_cache_on_failure)
//             os->wbinvd();
//         return status;
//     }

//     return OS_SUCCESS;
// }


OSSTATUS do_CMD_ALLOC_AND_LOCK_PAGES_FOR_SDOM(struct tz_cmd_packet *cmd_packet)
{
    uint32_t num_pages = (uint32_t)cmd_packet->arg1;    // arg1 - Number of 4K pages to be allocated and locked
    void* ptr = NULL;
    uvaddr_t uvaddr = INVALID_VADDR;

    ptr = os->mm_alloc_and_lock_pages_for_sdom(num_pages, &uvaddr);
    if(!ptr)
        return E_INTERNAL_ERROR;

    // Report the result user vaddr
    cmd_packet->arg1 = uvaddr;
    
    // On success
    return OS_SUCCESS;
}

OSSTATUS do_CMD_FREE_AND_UNLOCK_PAGES_FOR_SDOM(struct tz_cmd_packet *cmd_packet)
{
    uvaddr_t uvaddr = (uvaddr_t)cmd_packet->arg1;    // arg1 - Start of the memory region vaddr in the user mode

    os->mm_free_and_unlock_pages_for_sdom_by_uvaddr(uvaddr);

    // On success
    return OS_SUCCESS;
}