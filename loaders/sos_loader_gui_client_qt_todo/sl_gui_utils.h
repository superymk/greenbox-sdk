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

#ifndef SOSLOADER_GUI_UTILS_H
#define SOSLOADER_GUI_UTILS_H

extern "C"
{
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

    #define FS_MAX_PATH_BUFFER_LEN  (MAX_PATH_LEN + 1)  // +1 for the string terminator '\0'

    /// @brief Return the absolute path (excluding the file name) of the current executable
    /// @param  
    /// @return 
    char* apploader_get_exe_directory(void);

}

#endif // SOSLOADER_GUI_UTILS_H