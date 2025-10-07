// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

/* dummy lock routines for single-threaded aps */

typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;
 
#include <_ansi.h>

extern void __libc_lock_init(_LOCK_T *lock);
extern void __libc_lock_init_recursive(_LOCK_RECURSIVE_T *lock);
extern void __libc_lock_close(_LOCK_T *lock);
extern void __libc_lock_close_recursive(_LOCK_RECURSIVE_T *lock);
extern void __libc_lock_acquire(_LOCK_T *lock);
extern void __libc_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock);
extern void __libc_lock_release(_LOCK_T *lock);
extern void __libc_lock_release_recursive(_LOCK_RECURSIVE_T *lock);

/* Returns 0 for success and non-zero for failure */
extern int __libc_lock_try_acquire(_LOCK_T *lock);
extern int __libc_lock_try_acquire_recursive(_LOCK_RECURSIVE_T *lock);

#define __LOCK_INIT(class,lock) class int lock = 1;
#define __LOCK_INIT_RECURSIVE(class,lock) class int lock = 1;
#define __lock_init(lock) \
    __libc_lock_init(&(lock))
#define __lock_init_recursive(lock) \
    __libc_lock_init_recursive(&(lock))
#define __lock_close(lock) \
    __libc_lock_close(&(lock))
#define __lock_close_recursive(lock) \
    __libc_lock_close_recursive(&(lock))
#define __lock_acquire(lock) \
    __libc_lock_acquire(&(lock))
#define __lock_acquire_recursive(lock) \
    __libc_lock_acquire_recursive(&(lock))
#define __lock_try_acquire(lock) \
    __libc_lock_try_acquire(&(lock))
#define __lock_try_acquire_recursive(lock) \
    __libc_lock_try_acquire_recursive(&(lock))
#define __lock_release(lock) \
    __libc_lock_release(&(lock))
#define __lock_release_recursive(lock) \
    __libc_lock_release_recursive(&(lock))
#endif /* __SYS_LOCK_H__ */
