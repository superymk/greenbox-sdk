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

// #include <sys/lock.h>

.section .text

// Function: Read EBP
.global read_reg_bp
read_reg_bp:
    movq %rbp, %rax
    ret
    
// Copy <x86_spin_lock> and <x86_spin_unlock> from XMHF-v0.2.2
// Function: x86_spin_lock(lock_addr)
.global x86_spin_lock
x86_spin_lock:
    btl $0, (%rdi)
    jnc x86_spin_lock

    lock
    btrl $0, (%rdi)
    jnc x86_spin_lock

    ret

// Function: x86_spin_unlock(lock_addr)	
.global x86_spin_unlock
x86_spin_unlock:
    btsl $0, (%rdi)
    ret

#endif // __amd64__