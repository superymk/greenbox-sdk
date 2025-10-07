// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef MACHINE_AMD64_ATOMIC
#define MACHINE_AMD64_ATOMIC

#define LOCK_PREFIX "lock;"

#define atomic_increase(mem)	\
	if (sizeof (*mem) == 4)					      \
	      __asm __volatile (LOCK_PREFIX "incl %0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 8)				      \
	      __asm __volatile (LOCK_PREFIX "REX.W inc %0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      

#endif // MACHINE_I386_ATOMIC

