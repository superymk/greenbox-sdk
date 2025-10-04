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

#ifndef _APP_STDLIB_H
#define _APP_STDLIB_H

#ifndef __ASSEMBLY__

#include <stdatomic.h>

#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // min

#ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // max

typedef struct {
    atomic_flag flag;
} app_spinlock_t;

extern void app_spin_lock_init(volatile app_spinlock_t *lock);

extern void app_spin_lock(volatile app_spinlock_t* lock);

extern void app_spin_unlock(volatile app_spinlock_t* lock);
//extern void app_flush_tlb_allcores(void);
#endif //__ASSEMBLY__

#endif // _APP_STDLIB_H