// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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