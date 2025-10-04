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

#include <appddk.h>

#include <stdatomic.h>
#include <stdbool.h>

void app_spin_lock_init(volatile app_spinlock_t *lock)
{
    atomic_flag_clear(&lock->flag);
}

void app_spin_lock(volatile app_spinlock_t *lock)
{
    // Loop until we successfully set the flag from false to true
    while (atomic_flag_test_and_set_explicit(&lock->flag, memory_order_acquire))
    {
        // Optionally, add a pause or yield instruction to reduce CPU usage
        // For example, on GCC/Clang:
        __builtin_ia32_pause(); // x86-specific
    }
}

void app_spin_unlock(volatile app_spinlock_t *lock)
{
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
}