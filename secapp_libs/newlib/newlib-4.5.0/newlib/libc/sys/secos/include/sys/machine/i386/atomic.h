// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef MACHINE_I386_ATOMIC
#define MACHINE_I386_ATOMIC

#define LOCK_PREFIX "lock;"

#define atomic_increase(mem)    \
do {                            \
    if (sizeof (*mem) == 1)         \
        __asm __volatile (LOCK_PREFIX "incb %b0" \
                          : "=m" (*mem)          \
                          : "m" (*mem));         \
    else if (sizeof (*mem) == 2)    \
        __asm __volatile (LOCK_PREFIX "incw %w0" \
                          : "=m" (*mem)          \
                          : "m" (*mem));         \
    else if (sizeof (*mem) == 4)    \
        __asm __volatile (LOCK_PREFIX "incl %0"  \
                          : "=m" (*mem)          \
                          : "m" (*mem));         \
} while (0)

#define atomic_decrease(mem)    \
do {                            \
    if (sizeof (*mem) == 1)         \
        __asm __volatile (LOCK_PREFIX "decb %b0" \
                          : "=m" (*mem)          \
                          : "m" (*mem));         \
    else if (sizeof (*mem) == 2)    \
        __asm __volatile (LOCK_PREFIX "decw %w0" \
                          : "=m" (*mem)          \
                          : "m" (*mem));         \
    else if (sizeof (*mem) == 4)    \
        __asm __volatile (LOCK_PREFIX "decl %0"  \
                          : "=m" (*mem)          \
                          : "m" (*mem));         \
} while (0)

#endif // MACHINE_I386_ATOMIC
