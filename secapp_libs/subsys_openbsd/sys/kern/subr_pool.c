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


/*	$OpenBSD: subr_pool.c,v 1.236 2022/08/14 01:58:28 jsg Exp $	*/
/*	$NetBSD: subr_pool.c,v 1.61 2001/09/26 07:14:56 chs Exp $	*/

/*-
 * Copyright (c) 1997, 1999, 2000 The NetBSD Foundation, Inc.
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

#include <sddk.h>

// #define SKIP_OPENBSD_RENAME
#include <subsys_openbsd.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/pool.h>

// [GreenBox-Specific] Implement with GreenBox APIs.

/*
 * Initialize the given pool resource structure.
 *
 * We export this routine to allow other kernel parts to declare
 * static pools that must be initialized before malloc() is available.
 */
void
pool_init(struct pool *pp, size_t size, u_int align, int ipl, int flags,
    const char *wchan, struct pool_allocator *palloc)
{
    // [GreenBox-Specific] Modified for GreenBox APIs.
    size = roundup(size, align);

    pp->pr_size = size;
}

/*
 * Decommission a pool resource.
 */
void
pool_destroy(struct pool *pp)
{
    // [GreenBox-Specific] Dummy type, because it is unnecessary for GreenBox APIs.
    pp->pr_size = 0;
}

/*
 * Grab an item from the pool.
 */
void *
pool_get(struct pool *pp, int flags)
{
    return smalloc(MEM_HEAP, pp->pr_size);
}

/*
 * Return resource to the pool.
 */
void
pool_put(struct pool *pp, void *v)
{
    sfree(MEM_HEAP, v, pp->pr_size);
}

// [GreenBox-Specific] End