// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* Defines App - SecAgent Interfaces */

#ifndef APP_OS_INTERFACE_H
#define APP_OS_INTERFACE_H

#include <app_types.h>
#include <secagent.h>
#include "./hash/hash.h"

#ifndef __ASSEMBLY__

extern int app_lock_mem(void* ptr, size_t len);
extern int app_unlock_mem(const void* addr, size_t len);

/// @brief Create directories recursively. Ensure all directories in the path are created.
/// @param filepath 
/// @return 
extern int fs_create_directories(const char *filepath);

/// @brief Read the entire file into memory
/// @param filepath 
/// @param create_if_not_exist 
/// @param data 
/// @param len 
/// @return 
extern int fs_read_file(const char *filepath, bool create_if_not_exist, uint8_t **data, size_t *len);

/// @brief Write data to a file
/// @param filepath 
/// @param data 
/// @param len 
/// @return Return 0 on success. Return -1 on errors.
extern int fs_write_file(const char *filepath, const uint8_t *data, size_t len);

/// @brief Deletes a file at the specified filepath.
/// @param filepath 
/// @return Return 0 on success, -1 on failure.
extern int fs_delete_file(const char *filepath);

/// @brief Execute "chmod 0664" on a given file. The owner and group can read and write the file. Others can only read  
/// the file.
/// @param filepath 
/// @return Return - on success. Return -1 on errors.
extern int fs_chmod_file_0664(const char *filepath);

extern bool os_ioctl(struct tz_cmd_packet* cmd, int* err_no);;

/// @brief Execute a program as a child process and wait the process exit
/// @param ignore_child_status <true> if the caller wants to ignore the child process's return status. 
/// @param exe_filepath 
/// @param exe_file_hash [Optional] If not NULL, then the function only execute the program if its hash matches <exe_file_hash>.
/// @param exe_name 
/// @param first_arg The argument must not be NULL. 
/// @param  
/// @return Return 0 if the child process exits with the code 0. 
///         Return the child process's exit codes on its errors
///         Return -1 otherwise.
extern int os_execute_program_wait_exit(bool ignore_child_status, const char* exe_filepath, const union sha_digest* exe_file_hash, const char* exe_name, const char *first_arg, ...);

/// @brief Locate the first drive containing "uxmhfmhv.efi" in the root.
/// @param out_drive_str Must have length of <FS_MAX_PATH_BUFFER_LEN>
/// @return 
extern int uxmhf_mhv_locate_first_drive(char* out_drive_str);
#endif // __ASSEMBLY__
#endif // APP_OS_INTERFACE_H