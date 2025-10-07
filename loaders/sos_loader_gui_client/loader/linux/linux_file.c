// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <appddk.h>
#include "../include/loader_os_intfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define _XOPEN_SOURCE 700   // optional but nice on POSIX
#include <libgen.h> // required for dirname()
#include <sys/file.h>
#include <sys/stat.h>
#include <linux/limits.h>

int os_file_lock_create(const char* filepath, ulong_t* out_fd)
{
	int fd;

    // Open or create the file with read and write permissions for the owner only
    fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return -1;

    // Attempt to acquire an exclusive lock on the file
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) 
	{
        close(fd);
        return -1;
    }

    // Write something to the file
    const char *text = "Hello, world!\n";
    if (write(fd, text, strlen(text)) == -1) 
	{
        // Attempt to release the lock
        flock(fd, LOCK_UN);
        close(fd);
        return -1;
    }

	// On success
	*out_fd = (ulong_t)fd;
	return 0;
}

int os_file_lock_release(ulong_t fd)
{
    if (flock(fd, LOCK_UN) == -1) {
        close(fd);
        return -1;
    }

    close(fd);

	// On success
	return 0;
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

FILE* fopen_normalized_path(const char *filepath, const char* mode)
{
    char normPath[PATH_MAX] = {0};
    normalize_path(filepath, normPath);
    
    return fopen(normPath, mode);
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