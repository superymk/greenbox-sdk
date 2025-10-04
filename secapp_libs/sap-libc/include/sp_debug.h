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

#ifndef _SECPROCESS_DEBUG_H
#define _SECPROCESS_DEBUG_H



#ifndef __ASSEMBLY__

extern void debug_sp_print_memory(void* ptr, size_t sz);
extern void debug_sp_utask_dump(SP_UTASK* params);

extern uint32_t sos_sys_test(uint32_t arg1);

#endif // __ASSEMBLY__
#endif // _SECPROCESS_DEBUG_H