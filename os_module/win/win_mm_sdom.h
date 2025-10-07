// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_MEMORY_SDOM_H
#define SECAGENT_WIN_MEMORY_SDOM_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern void sa_s_mem_init(void);

extern void sa_s_mem_fini(void);

extern void* win_mm_alloc_and_lock_pages_for_sdom(uint32_t num_pages, uvaddr_t* out_uvaddr);
extern void win_mm_free_and_unlock_pages_for_sdom_by_kvaddr(void* ptr);
extern void win_mm_free_and_unlock_pages_for_sdom_by_uvaddr(uvaddr_t uvaddr);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_MEMORY_SDOM_H