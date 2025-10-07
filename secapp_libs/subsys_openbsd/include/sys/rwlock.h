/*	$OpenBSD: rwlock.h,v 1.28 2021/01/11 18:49:38 mpi Exp $	*/
/*
 * Copyright (c) 2002 Artur Grabowski <art@openbsd.org>
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

/*
 * Multiple readers, single writer lock.
 *
 * Simplistic implementation modelled after rw locks in Solaris.
 *
 * The rwl_owner has the following layout:
 * [ owner or count of readers | wrlock | wrwant | wait ]
 *
 * When the WAIT bit is set (bit 0), the lock has waiters sleeping on it.
 * When the WRWANT bit is set (bit 1), at least one waiter wants a write lock.
 * When the WRLOCK bit is set (bit 2) the lock is currently write-locked.
 *
 * When write locked, the upper bits contain the struct proc * pointer to
 * the writer, otherwise they count the number of readers.
 *
 * We provide a simple machine independent implementation:
 *
 * void rw_enter_read(struct rwlock *)
 *  atomically test for RWLOCK_WRLOCK and if not set, increment the lock
 *  by RWLOCK_READ_INCR. While RWLOCK_WRLOCK is set, loop into rw_enter_wait.
 *
 * void rw_enter_write(struct rwlock *);
 *  atomically test for the lock being 0 (it's not possible to have
 *  owner/read count unset and waiter bits set) and if 0 set the owner to
 *  the proc and RWLOCK_WRLOCK. While not zero, loop into rw_enter_wait.
 *
 * void rw_exit_read(struct rwlock *);
 *  atomically decrement lock by RWLOCK_READ_INCR and unset RWLOCK_WAIT and
 *  RWLOCK_WRWANT remembering the old value of lock and if RWLOCK_WAIT was set,
 *  call rw_exit_waiters with the old contents of the lock.
 *
 * void rw_exit_write(struct rwlock *);
 *  atomically swap the contents of the lock with 0 and if RWLOCK_WAIT was
 *  set, call rw_exit_waiters with the old contents of the lock.
 */

#ifndef _SYS_RWLOCK_H
#define _SYS_RWLOCK_H

// [GreenBox-Specific] Dummy type, because it is unnecessary for SecServices.
//// [NOTE] SecServices are not interruptable during its operations
struct rwlock {
    int dummy;
};
// [GreenBox-Specific] End.

void    rw_init(struct rwlock *, const char*);
void	rw_enter_read(struct rwlock *);
void	rw_enter_write(struct rwlock *);
void	rw_exit_read(struct rwlock *);
void	rw_exit_write(struct rwlock *);

#ifdef DIAGNOSTIC
void	rw_assert_wrlock(struct rwlock *);
void	rw_assert_rdlock(struct rwlock *);
void	rw_assert_anylock(struct rwlock *);
void	rw_assert_unlocked(struct rwlock *);
#else
#define rw_assert_wrlock(rwl)	((void)0)
#define rw_assert_rdlock(rwl)	((void)0)
#define rw_assert_anylock(rwl)	((void)0)
#define rw_assert_unlocked(rwl)	((void)0)
#endif

int	rw_enter(struct rwlock *, int);
void	rw_exit(struct rwlock *);

#endif /* _SYS_RWLOCK_H */
