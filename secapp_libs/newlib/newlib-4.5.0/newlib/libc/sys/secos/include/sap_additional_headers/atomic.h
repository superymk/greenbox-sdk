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

#ifndef SECAPP_X86_ATOMIC
#define SECAPP_X86_ATOMIC

#define LOCK_PREFIX "lock;"

static inline void sp_x86_atomic_set(uint32_t *var, uint32_t val)
{
    __asm__ __volatile__("xchgl %0, %1"
                         : "+r"(val), "+m"(*var)
                         :
                         : "memory");
    // After this, *var holds the new value; 'val' holds the old value (ignored).
}

/// SeqCst-style read using a locked RMW that leaves memory unchanged.
/// lock xadd with 0 returns the old value in the register.
static inline uint32_t sp_x86_atomic_get(const uint32_t *var)
{
    uint32_t zero = 0;  // will receive the old value
    __asm__ __volatile__(LOCK_PREFIX "xaddl %0, %1"
                         : "+r"(zero), "+m"(*(volatile uint32_t *)var)
                         :
                         : "cc", "memory");
    return zero;
}

static inline void sp_x86_atomic_increase(uint32_t *var)
{
    __asm __volatile(LOCK_PREFIX "incl %0"
                     : "=m"(*var)
                     : "m"(*var));
}

static inline void sp_x86_atomic_decrease(uint32_t *var)
{
    __asm __volatile(LOCK_PREFIX "decl %0"
                     : "=m"(*var)
                     : "m"(*var));
}

#endif // SECAPP_X86_ATOMIC
