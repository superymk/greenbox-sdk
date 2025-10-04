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

#include "sl_gui_utils.h"
#include "sl_gui_ctrl.h"
#include <string.h>
// #include <sl_headers.h>

#include <stdio.h>

extern "C"
{
    /*********** [TODO] Get Executable Directory  **********/
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #if defined(_WIN32) || defined(__WINDOWS__)
    #include <windows.h>
    #elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h>
    #else
    #include <unistd.h>
    #include <linux/limits.h>
    #endif

    #define MAX_PATH_OUTPUT_LEN	(MAX_PATH_LEN + 1) // Need one more character to append the last "directory delimiter"

    static char exePath[MAX_PATH_OUTPUT_LEN] = {0};
    char* apploader_get_exe_directory(void)
    {
        char* lastSlash;

        memset(exePath, 0, MAX_PATH_OUTPUT_LEN);

        // if (exePath == NULL) {
        //     printf("Memory allocation error");
        //     return NULL;
        // }

    #if defined(_WIN32) || defined(__WINDOWS__)
        // Windows specific
        wchar_t wideExePath[MAX_PATH_OUTPUT_LEN] = { 0 };

        // Use wide-character string function and buffer
        if (GetModuleFileNameW(NULL, wideExePath, MAX_PATH_OUTPUT_LEN) == 0) {
            printf("GetModuleFileName failed");
            return NULL;
        }

        // Convert wide character path back to multibyte character set
        if (WideCharToMultiByte(CP_ACP, 0, wideExePath, -1, exePath, MAX_PATH_OUTPUT_LEN, NULL, NULL) == 0) {
            printf("WideCharToMultiByte failed");
            return NULL;
        }

        // Extract the directory path
        lastSlash = strrchr(exePath, '\\');
        if (lastSlash != NULL) {
            *lastSlash = '\\';
            *(lastSlash + 1) = '\0';
        }
    #elif defined(__APPLE__)
        uint32_t bufsize = MAX_PATH_LEN;
        if (_NSGetExecutablePath(exePath, &bufsize) != 0) {
            printf("_NSGetExecutablePath failed");
            return NULL;
        }

        // Extract the directory path
        lastSlash = strrchr(exePath, '/');
        if (lastSlash != NULL) {
            *lastSlash = '/';
            *(lastSlash + 1) = '\0';
        }
    #else
        // Linux specific
        ssize_t count = readlink("/proc/self/exe", exePath, MAX_PATH_LEN);
        if (count == -1 || count >= MAX_PATH_LEN) {
            printf("readlink failed");
            return NULL;
        }

        // Null-terminate the string
        exePath[count] = '\0';

        // Extract the directory path
        lastSlash = strrchr(exePath, '/');
        if (lastSlash != NULL) {
            *lastSlash = '/';
            *(lastSlash + 1) = '\0';
        }
    #endif

        return exePath;
    }
}