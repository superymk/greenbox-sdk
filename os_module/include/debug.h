// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_DEBUG_H
#define SECAGENT_DEBUG_H
#include <os_types.h>


#ifndef __ASSEMBLY__

extern void dbg_dumpmem(vaddr_t start_vaddr, size_t size);


#endif // __ASSEMBLY__
#endif // SECAGENT_DEBUG_H