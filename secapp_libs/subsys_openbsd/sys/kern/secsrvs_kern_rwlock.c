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


/*	$OpenBSD: kern_rwlock.c,v 1.50 2023/07/14 07:07:08 claudio Exp $	*/

/*
 * Copyright (c) 2002, 2003 Artur Grabowski <art@openbsd.org>
 * Copyright (c) 2011 Thordur Bjornsson <thib@secnorth.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/rwlock.h>

// [GreenBox-Specific] SecServices do not need locks because they are always invoked with interrupts disabled.
void 
rw_init(struct rwlock *rwl, const char* name)
{
    // [GreenBox-Specific] Do nothing
    return;
}

void
rw_enter_read(struct rwlock *rwl)
{
    // [GreenBox-Specific] Do nothing
    return;
}

void
rw_enter_write(struct rwlock *rwl)
{
    // [GreenBox-Specific] Do nothing
    return;
}

void
rw_exit_read(struct rwlock *rwl)
{
    // [GreenBox-Specific] Do nothing
    return;
}

void
rw_exit_write(struct rwlock *rwl)
{
    // [GreenBox-Specific] Do nothing
    return;
}

int
rw_enter(struct rwlock *rwl, int flags)
{
    // [GreenBox-Specific] Do nothing
    return 0;
}

void
rw_exit(struct rwlock *rwl)
{
    // [GreenBox-Specific] Do nothing
    return;
}

// [GreenBox-Specific] End