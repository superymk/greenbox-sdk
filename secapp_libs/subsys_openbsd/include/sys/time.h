/*	$OpenBSD: time.h,v 1.32 2022/10/25 16:30:30 millert Exp $	*/
/*	$NetBSD: time.h,v 1.9 1994/10/26 00:56:35 cgd Exp $	*/

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)time.h	5.12 (Berkeley) 3/9/91
 */

#ifndef _TIME_H_
#define	_TIME_H_

#ifndef	_CLOCK_T_DEFINED_
#define	_CLOCK_T_DEFINED_
typedef	__clock_t	clock_t;
#endif

#ifndef	_TIME_T_DEFINED_
#define	_TIME_T_DEFINED_
typedef	__time_t	time_t;
#endif

#ifndef	_SIZE_T_DEFINED_
#define	_SIZE_T_DEFINED_
typedef	__size_t	size_t;
#endif

#if defined(_KERNEL) || defined(_STANDALONE)

#include <sys/stdint.h>

static inline uint64_t
SEC_TO_NSEC(uint64_t seconds)
{
	if (seconds > UINT64_MAX / 1000000000ULL)
		return UINT64_MAX;
	return seconds * 1000000000ULL;
}

static inline uint64_t
MSEC_TO_NSEC(uint64_t milliseconds)
{
	if (milliseconds > UINT64_MAX / 1000000ULL)
		return UINT64_MAX;
	return milliseconds * 1000000ULL;
}

static inline uint64_t
USEC_TO_NSEC(uint64_t microseconds)
{
	if (microseconds > UINT64_MAX / 1000ULL)
		return UINT64_MAX;
	return microseconds * 1000ULL;
}

#else /* !_KERNEL */
#endif /* !_KERNEL */

#endif /* !_TIME_H_ */
