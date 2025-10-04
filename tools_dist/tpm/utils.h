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

#ifndef TOOL_TPM_UTILS_H
#define TOOL_TPM_UTILS_H

#ifndef __ASSEMBLY__
#include <app_types.h>

#if defined(_WIN32) || defined(__WINDOWS__)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

#if defined(_WIN32) || defined(__WINDOWS__)
#define MAX_PATH_LEN MAX_PATH
#elif defined(__APPLE__)
#define MAX_PATH_LEN PATH_MAX
#else
#define MAX_PATH_LEN PATH_MAX
#endif

#define FS_MAX_PATH_BUFFER_LEN (MAX_PATH_LEN + 1) // +1 for the string terminator '\0'

extern int app_touch_mem(void *ptr, size_t len);

extern void print_hex(const char *label, const void *data, size_t length);


/*********** [TODO] Get Executable Directory  **********/

/// @brief Return the absolute file path of the executable's working directory.
/// @return 
extern char* apploader_get_exe_directory();

/// @brief Return the absolute path of the GreenBox binaries' root folder (e.g., "/home/superymk/greenbox/code/output/dist/")
/// [NOTE] This function cannot be ported to other GreenBox loaders, because they have different current working 
/// directories.
/// @param out_root_path 
extern void gb_get_root_absolute_path(char* out_root_path);

/// @brief Return the absolute path of the tpm-tools' user data (e.g., "/home/superymk/greenbox/code/output/dist/userdata/tools/tpmtools/")
/// [NOTE] This function cannot be ported to other GreenBox loaders, because they have different current working 
/// directories.
/// @param out_root_path 
extern void gb_get_tpmtools_userdata_absolute_path(char* out_userdata_tpm_path);

extern bool secagent_test_presence(void);

extern void press_continue(const char* prompt);

#endif // __ASSEMBLY__
#endif // TOOL_TPM_UTILS_H