// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SAP_SECPROCESS_H
#define SAP_SECPROCESS_H


#ifndef __ASSEMBLY__




extern void _dbg_print_backtrace_select(vaddr_t in_ebp, vaddr_t stack_top, vaddr_t stack_bottom);
extern void _dbg_print_backtrace(void);

#endif // __ASSEMBLY__
#endif // SAP_SECPROCESS_H