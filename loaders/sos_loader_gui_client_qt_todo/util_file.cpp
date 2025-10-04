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
                            printf("Timeout reached, child process is still running.\n");
                            return -2;
                        }
                        usleep(1000); // Sleep for 1ms to reduce CPU usage in the loop
                    }
                } while (result == 0);

                if (result == pid) {
                    // Child process exited, check exit status
                    if (WIFEXITED(status)) {
                        printf("Child exited with status %d\n", WEXITSTATUS(status));
                        if(status == 0)
                            return 0;
                        else
                            return -1;
                    } else if (WIFSIGNALED(status)) {
                        printf("Child terminated by signal %d\n", WTERMSIG(status));
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

char* FileUtils::getCurrentExecutablePath(void)
{
    return apploader_get_exe_directory();
}

SL_STATUS FileUtils::writeFile(const char* relative_filepath, uint8_t* file_buf, size_t file_size)
{
    char filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    FILE *file;
    size_t written = 0;

    // Check: Parameters must be valid
    if(!relative_filepath || !file_buf || !file_size)
        return SL_EINVAL;

    strncpy(filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(filepath, relative_filepath);

    // Open the file in binary write mode
    file = fopen_normalized_path(filepath, "wb");
    
    if (file == NULL) 
        return SL_EINVAL;

    // Write the data to the file
    written = fwrite(file_buf, 1, file_size, file);
    if(written != file_size)
        return SL_EINVAL;

    // Close the file
    fclose(file);

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