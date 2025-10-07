// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sys/lock.h>

static void _spin_lock(volatile _LOCK_T* lock)
{
	x86_spin_lock(lock);
}

static void _spin_unlock(volatile _LOCK_T* lock)
{
    asm volatile (""); // acts as a memory barrier.
    x86_spin_unlock(lock);
}

static void _spin_lock_init(volatile _LOCK_T* lock)
{
    asm volatile (""); // acts as a memory barrier.
    *lock = 1;
}

void __libc_lock_init(_LOCK_T *lock)
{
    _spin_lock_init(lock);
}

void __libc_lock_init_recursive(_LOCK_RECURSIVE_T *lock)
{
    return;
}

void __libc_lock_close(_LOCK_T *lock)
{
    _spin_unlock(lock);
}

void __libc_lock_close_recursive(_LOCK_RECURSIVE_T *lock)
{
    return;
}

void __libc_lock_acquire(_LOCK_T *lock)
{
    _spin_lock(lock);
}

void __libc_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    return;
}

void __libc_lock_release(_LOCK_T *lock)
{
    _spin_unlock(lock);
}

void __libc_lock_release_recursive(_LOCK_RECURSIVE_T *lock)\
{
    return;
}

int __libc_lock_try_acquire(_LOCK_T *lock)
{
    return -1;
}

int __libc_lock_try_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    return -1;
}
