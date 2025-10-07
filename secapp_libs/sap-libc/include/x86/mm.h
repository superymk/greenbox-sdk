// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_X86_MM_H
#define SECAPP_X86_MM_H



#ifndef __ASSEMBLY__

extern bool cache_flush_range(vaddr_t vaddr, size_t size);


#endif // __ASSEMBLY__
#endif // SECAPP_X86_MM_H