/*	$OpenBSD: pool.h,v 1.78 2021/01/02 03:23:59 cheloha Exp $	*/
/*	$NetBSD: pool.h,v 1.27 2001/06/06 22:00:17 rafal Exp $	*/

/*-
 * Copyright (c) 1997, 1998, 1999, 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Paul Kranenburg; by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
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

#ifndef _SYS_POOL_H_
#define _SYS_POOL_H_


// [GreenBox-Specific] Dummy type, because it is unnecessary for GreenBox APIs.
struct pool
{
    unsigned int	pr_size;	/* Size of item */
};

struct pool_allocator 
{
    int dummy;
};
// [GreenBox-Specific] End.

#define PR_WAITOK	0x0001 /* M_WAITOK */
#define PR_NOWAIT	0x0002 /* M_NOWAIT */
#define PR_LIMITFAIL	0x0004 /* M_CANFAIL */
#define PR_ZERO		0x0008 /* M_ZERO */
#define PR_RWLOCK	0x0010
#define PR_WANTED	0x0100


void		pool_init(struct pool *, size_t, u_int, int, int,
		    const char *, struct pool_allocator *);
void		pool_destroy(struct pool *);

void		*pool_get(struct pool *, int);
void		pool_put(struct pool *, void *);

#endif /* _SYS_POOL_H_ */
