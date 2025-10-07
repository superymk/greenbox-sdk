// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SHAREDMEM_MANAGER_H
#define APP_SHAREDMEM_MANAGER_H

#include <appddk.h>

#define INVALID_SMEM_OFFSET				(-1)

#define SMEM_OFFSET_TO_VADDR(offset)  	(smem_get_base() + (offset))
#define SMEM_VADDR_TO_OFFSET(vaddr)		(((vaddr_t)(vaddr)) - smem_get_base())
#ifndef __ASSEMBLY__

typedef int32_t 	smem_offset_t;

extern int smem_mgr_init(SP_USTRUCT* params);
extern void smem_mgr_fini(SP_USTRUCT* params);

extern vaddr_t smem_get_base(void);

extern smem_offset_t smem_allocate(size_t size);
extern void smem_free(smem_offset_t offset);

#endif // __ASSEMBLY__
#endif // APP_SHAREDMEM_MANAGER_H