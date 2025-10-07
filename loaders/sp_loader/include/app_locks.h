// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _SP_LOADER_LOCKS_H_
#define _SP_LOADER_LOCKS_H_

#include <pthread.h>

#define LOADER_MLOCK_T            pthread_mutex_t
#define LOADER_CREATE_LOCK(l)     pthread_mutex_init (l, NULL)
#define LOADER_DESTROY_LOCK(l)    pthread_mutex_destroy(l)
#define LOADER_ACQUIRE_LOCK(l)    pthread_mutex_lock(l)
#define LOADER_RELEASE_LOCK(l)    pthread_mutex_unlock(l)

#endif // _SP_LOADER_LOCKS_H_