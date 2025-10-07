// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_LIBC_OPS
#define SECAPP_LIBC_OPS

#include <sap_additional_headers/types.h>
#include <vk_errno.h>

#define LIBC_FATAL_ERR(format, ...) \
	do{	\
		printf("[LIBC][ERR][%s:%d] "format, __FILE__, __LINE__, ## __VA_ARGS__); sos_sys_sp_exit(E_NOT_IMPLEMENTED); } while(0)

extern void* sp_get_heap_base(void);

extern size_t sp_get_heap_size(void);

extern void* sp_get_sharedmem_base(void);

extern size_t sp_get_sharedmem_size(void);

extern uint32_t sp_get_sapid(void);
	
#endif // SECAPP_LIBC_OPS
