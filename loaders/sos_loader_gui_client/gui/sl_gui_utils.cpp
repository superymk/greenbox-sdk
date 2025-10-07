// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "sl_gui_utils.h"
#include "sl_gui_ctrl.h"
#include <string.h>
// #include <sl_headers.h>

#include <stdio.h>
#include "util_file.h"

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
        //     FileUtils::log("Memory allocation error");
        //     return NULL;
        // }

    #if defined(_WIN32) || defined(__WINDOWS__)
        // Windows specific
        wchar_t wideExePath[MAX_PATH_OUTPUT_LEN] = { 0 };

        // Use wide-character string function and buffer
        if (GetModuleFileNameW(NULL, wideExePath, MAX_PATH_OUTPUT_LEN) == 0) {
            FileUtils::log("GetModuleFileName failed");
            return NULL;
        }

        // Convert wide character path back to multibyte character set
        if (WideCharToMultiByte(CP_ACP, 0, wideExePath, -1, exePath, MAX_PATH_OUTPUT_LEN, NULL, NULL) == 0) {
            FileUtils::log("WideCharToMultiByte failed");
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
            FileUtils::log("_NSGetExecutablePath failed");
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
            FileUtils::log("readlink failed");
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

    bool str_equal(const char* s1, const char* s2)
    {
        size_t len1 = 0;
        size_t len2 = 0;
        
        if(!s1 || !s2)
            return false;

        len1 = strlen(s1);
        len2 = strlen(s2);

        if(len1 != len2)
            return false;

        if(strncmp(s1, s2, len1))
            return false;

        return true;
    }
}


#if defined(__WINDOWS__)
/**
 * @brief Executes a process and waits for it to complete.
 *
 * This function uses ShellExecuteExW to launch a process with the specified parameters
 * and waits indefinitely for the process to terminate.
 *
 * @param operation The operation to perform (e.g., "open", "runas").
 * @param file The file or executable to run.
 * @param parameters The command-line parameters to pass.
 * @param directory The working directory for the process.
 * @param showCmd The window display option (e.g., SW_SHOWNORMAL).
 * @return int The exit code of the process, or -1 on failure.
 */
int ShellExecuteAndWaitW(
    LPCWSTR operation,
    LPCWSTR file,
    LPCWSTR parameters,
    LPCWSTR directory,
    int showCmd
)
{
    SHELLEXECUTEINFOW sei = {0};
    sei.cbSize = sizeof(SHELLEXECUTEINFOW);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Obtain the process handle
    sei.hwnd = NULL;
    sei.lpVerb = operation;              // e.g., L"open", L"runas"
    sei.lpFile = file;                   // Path to the executable or document
    sei.lpParameters = parameters;       // Command-line parameters
    sei.lpDirectory = directory;         // Working directory
    sei.nShow = showCmd;                 // e.g., SW_SHOWNORMAL

    // Launch the process
    if (!ShellExecuteExW(&sei))
    {
        DWORD error = GetLastError();
        fwprintf(stderr, L"ShellExecuteExW failed with error code: %lu\n", error);
        return -1;
    }

    if (sei.hProcess == NULL)
    {
        fwprintf(stderr, L"ShellExecuteExW did not return a process handle.\n");
        return -1;
    }

    // Wait for the process to complete
    DWORD waitResult = WaitForSingleObject(sei.hProcess, INFINITE);
    if (waitResult == WAIT_FAILED)
    {
        DWORD error = GetLastError();
        fwprintf(stderr, L"WaitForSingleObject failed with error code: %lu\n", error);
        CloseHandle(sei.hProcess);
        return -1;
    }

    // Retrieve the exit code
    DWORD exitCode;
    if (!GetExitCodeProcess(sei.hProcess, &exitCode))
    {
        DWORD error = GetLastError();
        fwprintf(stderr, L"GetExitCodeProcess failed with error code: %lu\n", error);
        CloseHandle(sei.hProcess);
        return -1;
    }

    // Close the process handle
    CloseHandle(sei.hProcess);

    return static_cast<int>(exitCode);
}
#endif // __WINDOWS__