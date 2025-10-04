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

#ifndef SECAPP_LIBC_H
#define SECAPP_LIBC_H

#include <inttypes.h>
#include <stddef.h>

#ifndef __ASSEMBLY__

//! Init Libc
extern void libc_init(uint32_t sp_id, void* heap_base, size_t heap_size, void* ipc_app_base, 
	size_t ipc_app_size, void* sharedmem_base, size_t sharedmem_size);

//! Finalize Libc
extern void libc_fini(void);

#endif // __ASSEMBLY__
#endif // SECAPP_LIBC_H