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

#ifndef MACHINE_I386_ATOMIC
#define MACHINE_I386_ATOMIC

#define LOCK_PREFIX "lock;"

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

#endif // MACHINE_I386_ATOMIC

