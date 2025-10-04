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

/*	$OpenBSD: stddef.h,v 1.14 2017/01/06 14:36:50 kettenis Exp $	*/
/*	$NetBSD: stddef.h,v 1.4 1994/10/26 00:56:26 cgd Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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
 *	@(#)stddef.h	5.5 (Berkeley) 4/3/91
 */

#ifndef _STDDEF_H_
#define _STDDEF_H_

#include <sys/cdefs.h>
#include <sys/_null.h>
//  #include <sys/_types.h>

#define __SIZE_TYPE__ long unsigned int

#ifndef _PTRDIFF_T_DEFINED_
#define _PTRDIFF_T_DEFINED_
typedef long int __ptrdiff_t;
typedef __ptrdiff_t ptrdiff_t;
#endif

#ifndef _SIZE_T_DEFINED_
#define _SIZE_T_DEFINED_

#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__ __size_t;
#else
typedef long unsigned int __size_t;
#endif // __SIZE_TYPE__

typedef __size_t size_t;
#endif // _SIZE_T_DEFINED_

/* in C++, wchar_t is a built-in type */
#if !defined(_WCHAR_T_DEFINED_) && !defined(__cplusplus)
#define _WCHAR_T_DEFINED_
typedef int __wchar_t;
typedef __wchar_t wchar_t;
#endif

#ifndef _WINT_T_DEFINED_
#define _WINT_T_DEFINED_
typedef unsigned int __wint_t;
typedef __wint_t wint_t;
#endif

#if __GNUC_PREREQ__(4, 0)
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((size_t)(&((type *)0)->member))
#endif


#if __ISO_C_VISIBLE >= 2011 || __cplusplus >= 201103
#ifndef __CLANG_MAX_ALIGN_T_DEFINED
#define __CLANG_MAX_ALIGN_T_DEFINED
typedef struct
{
   long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
   long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
#endif
#endif

#endif /* _STDDEF_H_ */
