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

#ifdef __i386__

#include <sap_headers_libc.h>

ulong_t sp_sysenter(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3)
{
	ulong_t ret = 0;
	
	asm volatile ("movl %1, %%eax;"
		 "movl %2, %%ebx;"
		 "movl %3, %%esi;"
		 "movl %4, %%edi;"
		 "movl %%esp, %%ecx;"
		 "1: movl $1b, %%edx;"
		 "add $10, %%edx;" // sysenter return address points to the 1st inst after "sysenter"
		 "sysenter;" // go to ring 0
		 "movl %%eax, %0;" 
		 :"=m"(ret)
		 :"m"(fn), "m"(arg1), "m"(arg2), "m"(arg3)
		 :"%eax","%ebx","%esi","%edi", "%ecx", "%edx");	// clobbered registers
		 
	return ret;
}

#endif // __i386__