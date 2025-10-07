// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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