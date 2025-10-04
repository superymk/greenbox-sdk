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

/////// SecProcess Entry ///////
.section .text.entry

.extern arch_sp_entry

/*
 * The main entry of SecProcess. It is a trampoline to parse paramters passed by the SecOS into the calling convention
 * for the SecProcess under the target system architecture.
 * void sp_signal_handler
 */   
.global sp_entry
sp_entry:
	// SecOS ensures that %rsp is 16-byte aligned. %rsp must be 16 bytes alignment, see https://dev.haiku-os.org/ticket/10509
	xor		%rbp, %rbp					// Set %rbp to 0 as required by stack backtrace

	movq    0x8(%rsp), %rdi 			// param - <user_params>
	call	arch_sp_entry



/////// text Section ///////
.section .text

.extern sp_signal_handler_entry
.extern sos_sys_sp_signal_ret

/*
 * The main entry of PAL signal handler
 * void sp_signal_handler
 */   
.global sp_signal_handler
sp_signal_handler:
	movq    %rsp, %rax
	pushq 	%rax     // push %rsp
	xor		%rbp, %rbp					// Set %rbp to 0 as required by stack backtrace

    movq    0x10(%rax), %rdx // param - <info1>
    movq    0x8(%rax), %rsi // param - <is_intr>
	lea 	0x0(%rax), %rdi // param - <&sig_ret_ip>
   	call   	sp_signal_handler_entry
   	
   	popq 	%rax	// pop %rsp
    movq    %rax, %rdi // Read %esp for <sos_sys_sp_signal_ret>
   	call  	sos_sys_sp_signal_ret

	// never hit
   	ud2


#endif // __amd64__