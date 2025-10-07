// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifdef __i386__

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
	xor		%ebp, %ebp					// Set %ebp to 0 as required by stack backtrace

	jmp		arch_sp_entry



/////// text Section ///////
.section .text

.extern sp_signal_handler_entry
.extern sos_sys_sp_signal_ret

/*
 * The main entry of SecProcess signal handler
 * void sp_signal_handler
 */   
.global sp_signal_handler
sp_signal_handler:
	movl    %esp, %eax
	pushl 	%eax     // push %esp
	xor		%ebp, %ebp					// Set %ebp to 0 as required by stack backtrace

	pushl 	0x8(%eax) // param - <info1>
	pushl	0x4(%eax) // param - <is_intr>
	pushl 	%eax // param - <&sig_ret_ip>
   	call   	sp_signal_handler_entry
	popl    %eax  // discard parameter of <sp_signal_handler_entry>
   	popl    %eax  // discard parameter of <sp_signal_handler_entry>
	popl    %eax  // discard parameter of <sp_signal_handler_entry>
   	
   	popl 	%eax	// pop %esp
   	pushl  	%eax  // Read %esp for <sp_signal_handler>
   	call  	sos_sys_sp_signal_ret

	// never hit
   	ud2

#endif // __i386__