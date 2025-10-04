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

#ifndef _SP_LOADER_LOCKS_H_
#define _SP_LOADER_LOCKS_H_

#include <pthread.h>

#define LOADER_MLOCK_T            pthread_mutex_t
#define LOADER_CREATE_LOCK(l)     pthread_mutex_init (l, NULL)
#define LOADER_DESTROY_LOCK(l)    pthread_mutex_destroy(l)
#define LOADER_ACQUIRE_LOCK(l)    pthread_mutex_lock(l)
#define LOADER_RELEASE_LOCK(l)    pthread_mutex_unlock(l)

#endif // _SP_LOADER_LOCKS_H_