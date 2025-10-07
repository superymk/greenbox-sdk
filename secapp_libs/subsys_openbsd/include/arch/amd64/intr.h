/*	$OpenBSD: intr.h,v 1.33 2021/12/14 18:16:14 deraadt Exp $	*/
/*	$NetBSD: intr.h,v 1.2 2003/05/04 22:01:56 fvdl Exp $	*/

/*-
 * Copyright (c) 1998, 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum, and by Jason R. Thorpe.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE_INTR_H_
#define _MACHINE_INTR_H_

#include <machine/intrdefs.h>

/*
 * compiler barrier: prevent reordering of instructions.
 * This prevents the compiler from reordering code around
 * this "instruction", acting as a sequence point for code generation.
 */

#define	__splbarrier() __asm volatile("":::"memory")

/*
 * Hardware interrupt masks
 */
#define	splbio()	splraise(IPL_BIO)
#define	splnet()	splraise(IPL_NET)
#define	spltty()	splraise(IPL_TTY)
#define	splaudio()	splraise(IPL_AUDIO)
#define	splclock()	splraise(IPL_CLOCK)
#define	splstatclock()	splclock()
#define splipi()	splraise(IPL_IPI)

/*
 * Software interrupt masks
 */
#define	splsoftclock()	splraise(IPL_SOFTCLOCK)
#define	splsoftnet()	splraise(IPL_SOFTNET)
#define	splsofttty()	splraise(IPL_SOFTTTY)

/*
 * Miscellaneous
 */
#define	splvm()		splraise(IPL_VM)
#define	splhigh()	splraise(IPL_HIGH)
#define	spl0()		spllower(IPL_NONE)
#define	splsched()	splraise(IPL_SCHED)
#define	splx(x)		spllower(x)

/* SPL asserts */
#ifdef DIAGNOSTIC
/*
 * Although this function is implemented in MI code, it must be in this MD
 * header because we don't want this header to include MI includes.
 */
void splassert_fail(int, int, const char *);
extern int splassert_ctl;
void splassert_check(int, const char *);
#define splassert(__wantipl) do {			\
	if (splassert_ctl > 0) {			\
		splassert_check(__wantipl, __func__);	\
	}						\
} while (0)
#define splsoftassert(wantipl) splassert(wantipl)
#else
#define splassert(wantipl)	do { /* nada */ } while (0)
#define splsoftassert(wantipl)	do { /* nada */ } while (0)
#endif


int splraise(int);
int spllower(int);


#endif /* !_MACHINE_INTR_H_ */