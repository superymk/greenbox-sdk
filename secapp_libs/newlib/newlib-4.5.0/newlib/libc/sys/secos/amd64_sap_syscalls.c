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

#ifdef __amd64__

#include <sap_headers_libc.h>

ulong_t sp_sysenter(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3)
{
	ulong_t ret = 0;
	
	asm volatile ("movq %1, %%rax;"
		 "movq %2, %%rbx;"
		 "movq %3, %%rsi;"
		 "movq %4, %%rdi;"
		 "movq %%rsp, %%rcx;"
		 "1: movq $1b, %%rdx;"
		 "add $13, %%rdx;" // sysenter return address points to the 1st inst after "sysenter"
		 "sysenter;" // go to ring 0
		 "movq %%rax, %0;" 
		 :"=m"(ret)
		 :"m"(fn), "m"(arg1), "m"(arg2), "m"(arg3)
		 :"%rax","%rbx","%rsi","%rdi", "%rcx", "%rdx");	// clobbered registers
		 
	return ret;
}

#endif // __amd64__