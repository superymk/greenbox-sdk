// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifdef __i386__

 //#include <sys/lock.h>

.section .text

// Function: Read EBP
.global read_reg_bp
read_reg_bp:
    movl %ebp, %eax
    ret
    
// Copy <x86_spin_lock> and <x86_spin_unlock> from XMHF-v0.2.2
// Function: x86_spin_lock(lock_addr)
.global x86_spin_lock
x86_spin_lock:
    pushl %esi
    movl 0x8(%esp), %esi
    spin:	btl	$0, (%esi)		//mutex is available?
      	jnc	spin			      //wait till it is

      	lock				        //lock the bus (exclusive access)
        btrl	$0, (%esi)		    //and try to grab the mutex
        jnc	spin			      //spin until successful --> spinlock :p
    popl %esi
    ret

// Function: x86_spin_unlock(lock_addr)	
.global x86_spin_unlock
x86_spin_unlock:
    pushl %esi
    movl 0x8(%esp), %esi
    btsl	$0, (%esi)		       //release the spinlock
    popl %esi
    ret

#endif // __i386__