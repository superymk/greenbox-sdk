// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>

#include "lock.h"

/* Safe for scalar/pointer types. Don’t use on structs/arrays. */
#define READ_ONCE(x)  (*(const volatile __typeof__(x) *)&(x))
#define WRITE_ONCE(x, v) do { (*(volatile __typeof__(x) *)&(x)) = (v); } while (0)

void os_sos_spin_lock_init(volatile lock_t *lock)
{
    *lock = 1;
}

bool os_sos_spin_trylock(volatile lock_t* lock)
{
    unsigned char prev;
    asm volatile("lock; btrl $0, %1\n\t"
                 "setc %0"
                 : "=r"(prev), "+m"(*lock)
                 :
                 : "cc", "memory");
    return prev; /* prev==1 → we cleared 1→0 and acquired */
}

void os_sos_spin_lock(volatile lock_t* lock)
{
    for (;;) {
        /* Cheap test before issuing a locked RMW */
        if (READ_ONCE(*lock) & 1u) {
            if (os_sos_spin_trylock(lock))
                return;
        }
        /* Contended: spin politely until it looks free again */
        do {
            __builtin_ia32_pause(); // x86-specific
        } while (!(READ_ONCE(*lock) & 1u));
    }
}

void os_sos_spin_unlock(volatile lock_t* lock)
{
    asm volatile (""); // acts as a memory barrier.

    // locked RMW provides release semantics on x86
    asm volatile("lock; btsl $0, %0"
                 : "+m"(*lock)
                 :
                 : "cc", "memory");
}
