// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SP_LOADER_IPC_OS_SOS_SPINLOCK_H
#define SP_LOADER_IPC_OS_SOS_SPINLOCK_H

#include <ipc/app_comm.h>
#include <app_types.h>

#ifndef __ASSEMBLY__

/// @brief both i386 and amd64 uses 32-bit int for <lock_t>.
/// [NOTE] <lock_t> must have the same definition with it defined in SecOS.
typedef int lock_t;

void os_sos_spin_lock_init(volatile lock_t *lock);

bool os_sos_spin_trylock(volatile lock_t* lock);

void os_sos_spin_lock(volatile lock_t* lock);

void os_sos_spin_unlock(volatile lock_t* lock);


#endif // __ASSEMBLY__
#endif // SP_LOADER_IPC_OS_SOS_SPINLOCK_H