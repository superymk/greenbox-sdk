// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _SECPROCESS_DEBUG_H
#define _SECPROCESS_DEBUG_H



#ifndef __ASSEMBLY__

extern void sp_dbg_print_hex(const char *prefix, const void *prtptr, size_t size);
extern void debug_sp_print_memory(void* ptr, size_t sz);
extern void debug_sp_utask_dump(SP_UTASK* params);

extern uint32_t sos_sys_test(uint32_t arg1);

#endif // __ASSEMBLY__
#endif // _SECPROCESS_DEBUG_H