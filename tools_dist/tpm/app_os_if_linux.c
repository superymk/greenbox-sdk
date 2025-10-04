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

#ifdef __LINUX__

#define _GNU_SOURCE
#include <toolsddk.h>

#define _XOPEN_SOURCE 700   // optional but nice on POSIX
#include <stdio.h>
#include <fcntl.h>    // for open
#include <unistd.h>   // for close
#include <sys/mman.h> // for mmap
#include <errno.h>    // for errno
#include <stdlib.h>   // for malloc, free
#include <string.h>   /* memset */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <libgen.h> // required for dirname()
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <pthread.h>
#include <libgen.h>

#include <execinfo.h>
#include <sys/wait.h>

// #define __USE_GNU
#include <sched.h>
// #undef __USE_GNU

#include "utils.h"
#include "./hash/hash.h"

int app_lock_mem(void *ptr, size_t len)
{
    ptr = (void *)SIZE_ALIGN_4K((vaddr_t)ptr);
    len = SIZE_ROUNDUP_4K(len);

    if (!mlock(ptr, len))
    {
        app_touch_mem(ptr, len);
        return 0;
    }

    if (errno == ENOMEM)
    {
        /* try increasing soft limit.
         * this will only work if this process is privileged
         * or if the soft limit is less than the hard limit
         */
        struct rlimit memlock_limit;

        if (getrlimit(RLIMIT_MEMLOCK, &memlock_limit))
        {
            perror("getrlimit");
            return -1;
        }
        memlock_limit.rlim_cur += len;
        if (setrlimit(RLIMIT_MEMLOCK, &memlock_limit))
        {
            if (!(memlock_limit.rlim_cur > memlock_limit.rlim_max && errno == EINVAL))
            {
                perror("setrlimit");
            }
            return -2;
        }

        /* successfully increased limit. try locking again. */
        if (mlock(ptr, len))
        {
            perror("mlock");
            return -3;
        }

        app_touch_mem(ptr, len);
        return 0;
    }

    perror("mlock");
    return -4;
}

int app_unlock_mem(const void *addr, size_t len)
{
    return munlock(addr, len);
}

/// @brief Replaces every backslash in the path with a forward slash.
/// @param path 
static void convert_backslashes(char *path)
{
    for (char *p = path; *p; p++)
    {
        if (*p == '\\')
        {
            *p = '/';
        }
    }
}

/// @brief Converts an absolute path (which may include redundant "./" or "../" parts) into a canonical form.
/// This function:
///  - Converts backslashes to forward slashes.
///  - Splits the path on '/' (ignoring empty tokens).
///  - Skips "." tokens.
///  - Processes ".." tokens by removing the last valid token.
///  - Rebuilds the normalized absolute path.
/// @param path 
/// @param normalized 
static void normalize_path(const char *path, char *normalized)
{
    if (!path || !normalized)
    {
        return;
    }

    if (path[0] != '/')
    {
        fprintf(stderr, "Error: Only absolute paths are supported.\n");
        exit(EXIT_FAILURE);
    }

    // Make a working copy and convert backslashes to '/'
    char temp[PATH_MAX];
    strncpy(temp, path, PATH_MAX);
    temp[PATH_MAX - 1] = '\0';
    convert_backslashes(temp);

    // Array to store tokens (pointers within temp)
    char *stack[PATH_MAX];
    int stackSize = 0;

    char *token;
    char *saveptr;
    token = strtok_r(temp, "/", &saveptr);
    while (token != NULL)
    {
        if (strcmp(token, "") == 0 || strcmp(token, ".") == 0)
        {
            // Skip empty tokens or current-directory tokens.
        }
        else if (strcmp(token, "..") == 0)
        {
            if (stackSize > 0)
            {
                // Go up one level.
                stackSize--;
            }
        }
        else
        {
            stack[stackSize++] = token;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }

    // Rebuild the normalized path
    strcpy(normalized, "/");
    for (int i = 0; i < stackSize; i++)
    {
        strcat(normalized, stack[i]);
        if (i < stackSize - 1)
        {
            strcat(normalized, "/");
        }
    }
}

int fs_create_directories(const char *filepath)
{
    char normPath[PATH_MAX];
    normalize_path(filepath, normPath);

    // Work on a copy because dirname() may modify its input.
    char tempPath[PATH_MAX];
    strncpy(tempPath, normPath, PATH_MAX);
    tempPath[PATH_MAX - 1] = '\0';

    char *dir = dirname(tempPath);

    // Base case: nothing to create.
    if (strlen(dir) == 0 || strcmp(dir, ".") == 0 || strcmp(dir, "/") == 0)
    {
        return 0;
    }

    struct stat st;
    if (stat(dir, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return 0; // Directory exists.
        }
        else
        {
            fprintf(stderr, "Path %s exists but is not a directory\n", dir);
            return -1;
        }
    }

    // Recursively create parent directories.
    if (fs_create_directories(dir) != 0)
    {
        return -1;
    }

    if (mkdir(dir, 0777) != 0)
    { // Request full permissions; umask may modify this
        perror("mkdir");
        return -1;
    }

    // If running under sudo, adjust ownership so the original user owns the dir.
    const char *sudo_uid = getenv("SUDO_UID");
    const char *sudo_gid = getenv("SUDO_GID");
    if (sudo_uid && sudo_gid)
    {
        uid_t uid = (uid_t)atoi(sudo_uid);
        gid_t gid = (gid_t)atoi(sudo_gid);
        if (chown(dir, uid, gid) != 0)
        {
            perror("chown");
        }
    }

    // Explicitly set permissions so that all users can access and delete the folder.
    if (chmod(dir, 0777) != 0)
    {
        perror("chmod");
    }

    return 0;
}

int fs_read_file(const char *filepath, bool create_if_not_exist, uint8_t **data, size_t *len)
{
    int fd = 0;
    char normPath[PATH_MAX] = {0};

    normalize_path(filepath, normPath);

    if (create_if_not_exist)
    {
        // Mode: rw-r--r-- (owner can read/write, others can read)
        mode_t mode = 0644;
        fd = open(normPath, O_RDWR | O_CREAT | O_EXCL, mode);
    }
    else
        fd = open(normPath, O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0)
    {
        perror("fstat");
        close(fd);
        return -1;
    }

    *data = malloc(st.st_size);
    if (*data == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        close(fd);
        return -1;
    }

    if (read(fd, *data, st.st_size) != st.st_size)
    {
        perror("read");
        free(*data);
        close(fd);
        return -1;
    }

    *len = st.st_size;
    close(fd);
    return 0;
}

int fs_write_file(const char *filepath, const uint8_t *data, size_t len)
{
    char normPath[PATH_MAX] = {0};

    normalize_path(filepath, normPath);

    // Create directories if needed
    if (fs_create_directories(normPath) != 0)
    {
        return -1;
    }

    int fd = open(normPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    ssize_t written = write(fd, data, len);
    if (written < 0 || (size_t)written != len)
    {
        perror("write");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int fs_delete_file(const char *filepath)
{
    char normPath[PATH_MAX] = {0};

    normalize_path(filepath, normPath);

    // Attempt to delete the file
    if (remove(normPath) != 0)
    {
        // Determine the cause of the failure and print an appropriate message
        switch (errno)
        {
        case ENOENT:
            // File not exist
            return 0;
        case EACCES:
            fprintf(stderr, "Error deleting file '%s': Permission denied.\n", normPath);
            break;
        case EPERM:
            fprintf(stderr, "Error deleting file '%s': Operation not permitted.\n", normPath);
            break;
        case EISDIR:
            fprintf(stderr, "Error deleting file '%s': Is a directory.\n", normPath);
            break;
        default:
            fprintf(stderr, "Error deleting file '%s': %s.\n", normPath, strerror(errno));
            break;
        }
        return -1;
    }

    // Deletion successful
    return 0;
}

int fs_chmod_file_0664(const char *filepath)
{
    char normPath[PATH_MAX] = {0};

    normalize_path(filepath, normPath);

    // Set the file permissions to read and write for owner and group, and read for others
    if (chmod(normPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH) != 0)
    {
        perror("chmod failed");
        return -1;
    }

    return 0;
}

bool os_ioctl(struct tz_cmd_packet *cmd, int *err_no)
{
    /* call kernel module to allocate a physically-contiguous
     * non-cacheable physical memory pool */
    int fd = 0;
    int ret = 0;
    bool status = false;

    /* Open Device */
    fd = open(TZ_CLIENT_FULL_PATH_DEV_NAME, O_RDWR | O_SYNC);
    if (fd < 0)
    {
        printf("Cannot Open Device: %s\n", TZ_CLIENT_FULL_PATH_DEV_NAME);
        goto out;
    }

    ret = ioctl(fd, 1, cmd);
    if (ret)
    {
        printf("os_ioctl error with code %d!\n", ret);
        close(fd);
        goto out;
    }

    close(fd);
    status = true;

out:
    if (err_no)
        *err_no = ret;

    // printf("[Superymk] <os_ioctl> Finished cmd:%u\n", cmd->id);
    return status;
}

int os_execute_program_wait_exit(bool ignore_child_status, const char *exe_filepath, const union sha_digest *exe_file_hash, const char *exe_name, const char *first_arg, ...)
{
    pid_t pid = 0;

    // Check: params must be valid
    if (!exe_filepath || !exe_name)
        return -1;

    // [Optional] Check: the exe file matches its hash
    if (exe_file_hash)
    {
        uint8_t *file_data;
        size_t file_size;
        if (fs_read_file(exe_filepath, false, &file_data, &file_size) != 0)
        {
            return -1;
        }

        uint8_t file_hash[SHA256_DIGEST_LENGTH];
        if (sha2_256_mem(file_data, file_size, file_hash) != 0)
        {
            free(file_data);
            perror("Hash computation failed\n");
            return -1;
        }

        free(file_data);

        if (memcmp(file_hash, exe_file_hash->sha2_256_digest, SHA256_DIGEST_LENGTH) != 0)
        {
            perror("Executable file hash does not match\n");
            return -1;
        }
    }

    pid = fork();
    if (pid == -1)
    {
        // Fork failed
        perror("fork");
        return -1; // Return an error status
    }
    else if (pid == 0)
    {
        // Child process

        // Prepare arguments for execl
        va_list args;
        va_start(args, first_arg);

        // Assuming we have a limit on the number of arguments
        const int MAX_ARGS = 128;
        char *argv[MAX_ARGS];
        int argc = 0;

        // exe_name is the first argument according to execl conventions
        argv[argc++] = strdup(exe_name);
        argv[argc++] = strdup(first_arg);

        const char *temp_arg;
        while ((temp_arg = va_arg(args, const char *)) != NULL && argc < MAX_ARGS - 1)
        {
            argv[argc++] = strdup(temp_arg);
        }
        argv[argc] = NULL; // NULL-terminate the array

        // Execute the program
        execv(exe_filepath, argv);

        // If execv returns, it means there was an error
        perror("execv");
        exit(EXIT_FAILURE); // Terminate the child process
    }
    else
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish

        if (WIFEXITED(status))
        {
            if (ignore_child_status)
                return 0;
            else
                // Return the exit status of the child process
                return WEXITSTATUS(status);
        }
        else
        {
            // Child did not exit normally
            return -1; // Return an error status
        }
    }
}

int uxmhf_mhv_locate_first_drive(char* out_drive_str)
{
    printf("<uxmhf_mhv_locate_first_drive> unimplemented!\n");
    return -1;
}
#endif // __LINUX__