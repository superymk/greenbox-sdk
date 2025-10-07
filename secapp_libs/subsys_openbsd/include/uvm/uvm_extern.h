// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*	$OpenBSD: apmprobe.c,v 1.19 2016/06/10 18:36:06 jcs Exp $	*/

/*
 * Copyright (c) 1997-2000 Michael Shalayeff
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * APM derived from: apm_init.S, LP (Laptop Package)
 * which contained this:
 * Copyright (C) 1994 by HOSOKAWA, Tatsumi <hosokawa@mt.cs.keio.ac.jp>
 *
 */
/*
 * If you want to know the specification of APM BIOS, see the following
 * documentations,
 *
 * [1] Intel Corporation and Microsoft Corporation, "Advanced Power
 *     Management, The Next Generation, Version 1.0", Feb.,1992.
 *
 * [2] Intel Corporation and Microsoft Corporation, "Advanced Power
 *     Management (APM) BIOS Interface Specification Revision 1.1",
 *     Sep.,1993, Intel Order Number: 241704-001, Microsoft Part
 *     Number: 781-110-X01
 *
 * or contact
 *
 * APM Support Desk (Intel Corporation, US)
 *   TEL: (800)628-8686
 *   FAX: (916)356-6100.
 */

#define PAGE_MASK   0xfff
#define round_page(x)   (((x) + PAGE_MASK) & ~PAGE_MASK)