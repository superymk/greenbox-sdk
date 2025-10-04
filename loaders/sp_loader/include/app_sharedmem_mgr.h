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