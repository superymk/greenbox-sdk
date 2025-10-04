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

#include "util_file.h"
#include "sl_gui_utils.h"
// #include <sl_headers.h>

#include <stdio.h>

extern "C"
{
    #include "../loader/include/loader_os_intfs.h"
    
    #if defined(__WINDOWS__)
        #include <windows.h>
    #else
        #include <stdio.h>
        #include <stdlib.h>
        #include <unistd.h>
        #include <sys/types.h>
        #include <sys/wait.h>
        #include <time.h>
        #include <signal.h>

        int os_execute_program(uint32_t wait_child_exit_ms, const char* exe_filepath, const char* exe_name, const char *first_arg, ...)
        {
            pid_t pid = fork();

            if (pid == -1) {
                // Fork failed
                perror("fork");
                return -1; // Return an error status
            } else if (pid == 0) {
                // Child process

                // Prepare arguments for execl
                va_list args;
                va_start(args, first_arg);

                // Assuming we have a limit on the number of arguments
                const int MAX_ARGS = 128;
                char* argv[MAX_ARGS];
                int argc = 0;

                // exe_name is the first argument according to execl conventions
                argv[argc++] = strdup(exe_name);
                argv[argc++] = strdup(first_arg);

                const char* temp_arg;
                while ((temp_arg = va_arg(args, const char*)) != NULL && argc < MAX_ARGS - 1) {
                    argv[argc++] = strdup(temp_arg);
                }
                argv[argc] = NULL; // NULL-terminate the array

                // Execute the program
                execv(exe_filepath, argv);

                // If execv returns, it means there was an error
                perror("execv");
                exit(EXIT_FAILURE); // Terminate the child process
            } else {
                // Parent process
                // Wait the child process running for a maximum of <wait_child_exit_ms> ms.
                // If timeout, return -2. If the child process exits before then and its exit code is not 0, then return -1.
                struct timespec start_time, current_time;
                clock_gettime(CLOCK_MONOTONIC, &start_time);

                int status;
                pid_t result;
                do {
                    result = waitpid(pid, &status, WNOHANG); // Non-blocking wait
                    if (result == 0) {
                        // Child still running, check if timeout exceeded
                        clock_gettime(CLOCK_MONOTONIC, &current_time);
                        long elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000 + (current_time.tv_nsec - start_time.tv_nsec) / 1000000;
                        if (elapsed_ms > wait_child_exit_ms) {
                            FileUtils::log("Timeout reached, child process is still running.\n");
                            return -2;
                        }
                        usleep(1000); // Sleep for 1ms to reduce CPU usage in the loop
                    }
                } while (result == 0);

                if (result == pid) {
                    // Child process exited, check exit status
                    if (WIFEXITED(status)) {
                        FileUtils::log("Child exited with status %d\n", WEXITSTATUS(status));
                        if(status == 0)
                            return 0;
                        else
                            return -1;
                    } else if (WIFSIGNALED(status)) {
                        FileUtils::log("Child terminated by signal %d\n", WTERMSIG(status));
                        return -1;
                    }
                } else {
                    // An error occurred in waitpid
                    perror("waitpid");
                    return -1;
                }

                // return 0;
            }
        }
    #endif
}

static char log_filepath[MAX_PATH_LEN] = {0};
static bool log_filepath_initialized = false;

char* FileUtils::getCurrentExecutablePath(void)
{
    return apploader_get_exe_directory();
}

SL_STATUS FileUtils::greenbox_get_root_path(char* out_path)
{
    if(!out_path)
        return SL_EINVAL;

    strncpy(out_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    // SOSLoader_GUI is in the <root folder>, so we do not need to modify <out_path>.
    // strcat(out_path, "\\..\\");

    // On success
    return SL_SUCCESS;
}

SL_STATUS FileUtils::readFile(const char* filepath, bool create_if_not_exist, uint8_t **data, size_t *len)
{
    int ret = 0;

    ret = fs_read_file(filepath, create_if_not_exist, data, len);
    if(ret)
        return SL_EINTERNAL;

    // On success
    return SL_SUCCESS; 
}

SL_STATUS FileUtils::writeFile(const char* filepath, uint8_t* file_buf, size_t file_size)
{
    int ret = 0;

    ret = fs_write_file(filepath, file_buf, file_size);
    if(ret)
        return SL_EINTERNAL;

    // On success
    return SL_SUCCESS; 
}

SL_STATUS FileUtils::file_hash_and_check(const char* filepath, const union sha_digest *file_hash)
{
    SL_STATUS status = SL_SUCCESS;
    uint8_t *file_data = NULL;
    size_t file_size = 0;
    uint8_t calc_file_hash[SHA256_DIGEST_LENGTH];

    status = readFile(filepath, false, &file_data, &file_size);
    if(SL_FAIL(status))
        return SL_EINTERNAL;

    if (sha2_256_mem(file_data, file_size, calc_file_hash) != 0)
    {
        free(file_data);
        return SL_EINTERNAL;
    }

    free(file_data);

    if (memcmp(calc_file_hash, file_hash->sha2_256_digest, SHA256_DIGEST_LENGTH) != 0)
    {
        return SL_EINVAL;
    }

    // On success
    return SL_SUCCESS;
}

bool FileUtils::isFileExist(const char* relative_filepath)
{
    char filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    if(!relative_filepath)
        return false;

    // Open the file in binary read mode
    strncpy(filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(filepath, relative_filepath);
    
#if defined(_MSC_VER)
    return access(filepath, 0) == 0;
#else
    return access(filepath, F_OK) == 0;
#endif
}

SL_STATUS FileUtils::log(const char *format, ...)
{
    // Check: The log subsystem must be initialized.
    if(!log_filepath_initialized)
        return SL_EINVAL;

    // Open the log file in append mode
    FILE *log_file = fopen_normalized_path(log_filepath, "a");
    if (log_file == NULL)
    {
        return SL_EINVAL;
    }

    // Get the current time
    time_t now = time(NULL);
    if (now == ((time_t)-1))
    {
        fclose(log_file);
        return SL_EINVAL;
    }

    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        fclose(log_file);
        return SL_EINVAL;
    }

    // Write timestamp to the log file
    if (fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d ",
                t->tm_year + 1900,
                t->tm_mon + 1,
                t->tm_mday,
                t->tm_hour,
                t->tm_min,
                t->tm_sec) < 0)
    {
        fclose(log_file);
        return SL_EINVAL;
    }

    // Handle the variable arguments for the formatted message
    va_list args;
    va_start(args, format);
    if (vfprintf(log_file, format, args) < 0)
    {
        va_end(args);
        fclose(log_file);
        return SL_EINVAL;
    }
    va_end(args);

    // Close the log file
    if (fclose(log_file) != 0)
    {
        return SL_EINVAL;
    }

    return SL_SUCCESS;
}

/********* Public Functions - Logs *********/
SL_STATUS FileUtils::log_init(void)
{
    SL_STATUS status = SL_SUCCESS;

    status = FileUtils::greenbox_get_root_path(log_filepath);
    if(SL_FAIL(status))
        return status;
    strcat(log_filepath, SL_GUI_LOG_ROOT_RELATIVE_FILEPATH);
    log_filepath_initialized = true;

    // Attempt to delete the log file
    if (remove(log_filepath) != 0)
    {
        if (errno == ENOENT)
        {
            // File does not exist; continue the init.
        }
        else
        {
            // An error occurred while trying to delete the file.
            return SL_EIO;
        }
    }

    // Create directories of the file if needed
    if (fs_create_directories(log_filepath) != 0)
    {
        // Create error
        return SL_EIO;
    }
    
    // On success
    return SL_SUCCESS;
}

void FileUtils::log_fini(void)
{
    log_filepath_initialized = false;
}

extern "C" SL_STATUS FileUtils_Log(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // format into a local buffer
    char buf[1024] = { 0 };
    (void)vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    SL_STATUS status = FileUtils::log(buf);
    return status;
}




/********* OS Specific Functions - Windows *********/
#if defined(__WINDOWS__)
#include <windows.h>

SL_STATUS FileUtils::win_is_filepath_equal(const char* absolute_filepath1, const char* absolute_filepath2, bool* out_result)
{
    // Example paths (they may be written with "..", etc.)
    const char *path1 = absolute_filepath1;
    const char *path2 = absolute_filepath2;

    // Check: params must be valid
    if(!absolute_filepath1 || !absolute_filepath2 || !out_result)
        return SL_EINVAL;

    // Open the first file.
    HANDLE hFile1 = CreateFileA(
        path1,                    // file name
        GENERIC_READ,             // desired access
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
        NULL,                     // security attributes
        OPEN_EXISTING,            // creation disposition
        FILE_ATTRIBUTE_NORMAL,    // flags and attributes
        NULL);                    // template file

    if (hFile1 == INVALID_HANDLE_VALUE) {
        FileUtils::log("Could not open file 1 (%s): %lu\n", path1, GetLastError());
        return SL_EINVAL;
    }

    // Open the second file.
    HANDLE hFile2 = CreateFileA(
        path2,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile2 == INVALID_HANDLE_VALUE) {
        FileUtils::log("Could not open file 2 (%s): %lu\n", path2, GetLastError());
        CloseHandle(hFile1);
        return SL_EINVAL;
    }

    // Retrieve file information for both files.
    BY_HANDLE_FILE_INFORMATION info1;
    BY_HANDLE_FILE_INFORMATION info2;

    if (!GetFileInformationByHandle(hFile1, &info1)) {
        FileUtils::log("Error getting file info for file 1: %lu\n", GetLastError());
        CloseHandle(hFile1);
        CloseHandle(hFile2);
        return SL_EINTERNAL;
    }
    if (!GetFileInformationByHandle(hFile2, &info2)) {
        FileUtils::log("Error getting file info for file 2: %lu\n", GetLastError());
        CloseHandle(hFile1);
        CloseHandle(hFile2);
        return SL_EINTERNAL;
    }

    // Clean up file handles.
    CloseHandle(hFile1);
    CloseHandle(hFile2);

    // Compare the volume serial numbers and file indices.
    if (info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
        info1.nFileIndexHigh == info2.nFileIndexHigh &&
        info1.nFileIndexLow == info2.nFileIndexLow) 
    {
        *out_result = true;
    } 
    else 
    {
        *out_result = false;
    }

    // On success
    return SL_SUCCESS;
}
#endif // __WINDOWS__