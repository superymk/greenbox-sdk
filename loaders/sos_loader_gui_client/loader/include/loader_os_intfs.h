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

#include <os/os_headers.h>
#include <app_types.h>

#include <stdio.h>


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

extern int fs_create_directories(const char *filepath);
extern FILE* fopen_normalized_path(const char *filepath, const char* mode);
extern int fs_read_file(const char *filepath, bool create_if_not_exist, uint8_t **data, size_t *len);
extern int fs_write_file(const char *filepath, const uint8_t *data, size_t len);
extern int fs_delete_file(const char *filepath);
extern int fs_chmod_file_0664(const char *filepath);


#endif // __ASSEMBLY__
#endif // LOADER_OS_INTFS_H