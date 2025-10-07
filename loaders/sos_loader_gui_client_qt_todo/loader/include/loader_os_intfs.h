// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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