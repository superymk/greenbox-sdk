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

#ifndef LOADER_OS_INTFS_H
#define LOADER_OS_INTFS_H

#include <app_types.h>

#ifndef __ASSEMBLY__

/// @brief Create a lock on the file system 
/// @param filepath 
/// @param out_fd 
/// @return 
extern int os_file_lock_create(const char* filepath, ulong_t* out_fd);

/// @brief Release a lock on the file system.
/// @param fd 
/// @return 
extern int os_file_lock_release(ulong_t fd);

#endif // __ASSEMBLY__
#endif // LOADER_OS_INTFS_H