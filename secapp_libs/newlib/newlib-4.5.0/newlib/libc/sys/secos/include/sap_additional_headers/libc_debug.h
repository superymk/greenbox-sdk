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

#ifndef SAP_SECPROCESS_H
#define SAP_SECPROCESS_H


#ifndef __ASSEMBLY__




extern void _dbg_print_backtrace_select(vaddr_t in_ebp, vaddr_t stack_top, vaddr_t stack_bottom);
extern void _dbg_print_backtrace(void);

#endif // __ASSEMBLY__
#endif // SAP_SECPROCESS_H