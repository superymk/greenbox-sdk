// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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
    extern char* apploader_get_exe_directory(void);

    extern bool str_equal(const char* s1, const char* s2);

#if defined(__WINDOWS__)
    /// @brief Executes a process and waits for it to complete.
    /// This function uses ShellExecuteExW to launch a process with the specified parameters
    /// and waits indefinitely for the process to terminate.
    /// @param operation The operation to perform (e.g., "open", "runas").
    /// @param file The file or executable to run.
    /// @param parameters The command-line parameters to pass.
    /// @param directory The working directory for the process.
    /// @param showCmd The window display option (e.g., SW_SHOWNORMAL).
    /// @return int The exit code of the process, or -1 on failure.
    extern int ShellExecuteAndWaitW(
        LPCWSTR operation,
        LPCWSTR file,
        LPCWSTR parameters,
        LPCWSTR directory,
        int showCmd
    );
#endif // __WINDOWS__
}

#endif // SOSLOADER_GUI_UTILS_H