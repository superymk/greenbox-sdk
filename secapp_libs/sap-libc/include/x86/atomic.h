// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_X86_ATOMIC_H
#define SECAPP_X86_ATOMIC_H

#define LOCK_PREFIX "lock;"


#ifndef __ASSEMBLY__
#define atomic_increase(mem)	\
	if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK_PREFIX "incb %b0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK_PREFIX "incw %w0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK_PREFIX "incl %0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));	

#endif // __ASSEMBLY__
#endif // SECAPP_X86_ATOMIC_H