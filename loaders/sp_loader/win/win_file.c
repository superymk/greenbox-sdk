// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
#include <appddk.h>

#include <windows.h>
#include <stdio.h>



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

int os_file_lock_create(const char* filepath, ulong_t* out_fd)
{
    HANDLE hFile;

    // Open or create the file exclusively with read/write permissions
    hFile = CreateFile(
        filepath,                // File name
        GENERIC_READ | GENERIC_WRITE, // Open for reading and writing
        0,                       // Share mode: No sharing
        NULL,                    // Default security attributes
        CREATE_ALWAYS,           // Creates a new file, always
        FILE_ATTRIBUTE_NORMAL,   // Normal file attributes
        NULL);                   // No template file

    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    // Attempt to acquire an exclusive lock on the file
    OVERLAPPED overlapped = {0}; // For asynchronous operation
    if (!LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, MAXDWORD, MAXDWORD, &overlapped)) {
        CloseHandle(hFile);
        return -1;
    }

    // Write something to the file
    const char *text = "Hello, world!\n";
    DWORD bytesWritten;
    if (!WriteFile(hFile, text, strlen(text), &bytesWritten, NULL)) {
        // Attempt to release the lock
        UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &overlapped);
        CloseHandle(hFile);
        return -1;
    }

    // On success, return the file handle
    *out_fd = (ulong_t)hFile;
    return 0;
}

int os_file_lock_release(ulong_t fd)
{
    HANDLE hFile = (HANDLE)fd;
    OVERLAPPED overlapped = {0}; // For asynchronous operation

    if (!UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &overlapped)) {
        CloseHandle(hFile);
        return -1;
    }

    CloseHandle(hFile);

    // On success
    return 0;
}

int fs_create_directories(const char *filepath)
{
    char tempPath[PATH_MAX];
    /* Copy the input path into a temporary buffer */
    strncpy(tempPath, filepath, PATH_MAX - 1);
    tempPath[PATH_MAX - 1] = '\0'; // Ensure null termination

    /* Find the last directory separator */
    char *last_sep = NULL;
    for (char *p = tempPath; *p; ++p)
    {
        if (*p == '/' || *p == '\\')
        {
            last_sep = p;
        }
    }

    if (last_sep)
    {
        /* Temporarily truncate the string right after the last separator */
        char saved_char = *(last_sep + 1);
        *(last_sep + 1) = '\0';

        /* Walk the path creating each directory component */
        for (char *p = tempPath; *p; ++p)
        {
            if (*p == '/' || *p == '\\')
            {
                char old = *p;
                *p = '\0';

                /* Skip drive letters (e.g. "C:") if present */
                if (!(strlen(tempPath) == 2 && isalpha((unsigned char)tempPath[0]) && tempPath[1] == ':'))
                {
                    /* mkdir returns 0 on success or -1 on error.
                     * If the directory already exists (errno == EEXIST) we ignore it.
                     */
                    if (mkdir(tempPath, 0777) != 0 && errno != EEXIST)
                    {
                        fprintf(stderr, "Error creating directory %s: %s\n", tempPath, strerror(errno));
                        *(last_sep + 1) = saved_char;
                        return -1;
                    }
                }
                *p = old;
            }
        }
        *(last_sep + 1) = saved_char;
    }
    else
    {
        /* No directory separators found */
        return 0;
    }
    return 0;
}

FILE* fopen_normalized_path(const char *filepath, const char* mode)
{
    return fopen(filepath, mode);
}

int fs_read_file(const char *filepath, uint8_t create_if_not_exist, uint8_t **data, size_t *len)
{
    int fd;
    if (create_if_not_exist)
        fd = open(filepath, O_RDWR | O_CREAT, 0666);
    else
        fd = open(filepath, O_RDONLY);

    if (fd < 0) {
        fprintf(stderr, "Error opening file %s: %s\n", filepath, strerror(errno));
        return -1;
    }

    /* Obtain the file size using fstat() */
    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "Error getting file size for %s: %s\n", filepath, strerror(errno));
        close(fd);
        return -1;
    }
    off_t filesize = st.st_size;

    *data = malloc(filesize);
    if (*data == NULL && filesize > 0) {
        fprintf(stderr, "Memory allocation failed for file %s\n", filepath);
        close(fd);
        return -1;
    }

    ssize_t bytesRead = read(fd, *data, filesize);
    if (bytesRead < 0 || (size_t)bytesRead != filesize) {
        fprintf(stderr, "Error reading file %s: %s\n", filepath, strerror(errno));
        free(*data);
        close(fd);
        return -1;
    }

    *len = (size_t)filesize;
    close(fd);
    return 0;
}

int fs_write_file(const char *filepath, const uint8_t *data, size_t len)
{
    /* Create parent directories if needed */
    if (fs_create_directories(filepath) != 0)
        return -1;

    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        fprintf(stderr, "Error creating file %s: %s\n", filepath, strerror(errno));
        return -1;
    }

    ssize_t bytesWritten = write(fd, data, len);
    if (bytesWritten < 0 || (size_t)bytesWritten != len) {
        fprintf(stderr, "Error writing to file %s: %s\n", filepath, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int fs_delete_file(const char *filepath)
{
    if (unlink(filepath) != 0) {
        if (errno == ENOENT)
            return 0;  // File does not exist is not an error
        fprintf(stderr, "Error deleting file %s: %s\n", filepath, strerror(errno));
        return -1;
    }
    return 0;
}

int fs_file_get_size(const char *filename, size_t *out_sz)
{
	HANDLE hFile;
	uint32_t size_low, size_high;

	if (!out_sz)
		return -1;

	hFile = CreateFile(filename,
					   GENERIC_READ | GENERIC_WRITE,
					   0,
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		APP_ERR("Failed to get file size: open file error! %s\n", filename);
		return -1;
	}

	size_low = GetFileSize(hFile, (LPDWORD)&size_high);

	*out_sz = uint32_to_ulong(size_low, size_high);
	CloseHandle(hFile);

	return 0;
}

/* Set file permissions to 0664 (rw-rw-r--). */
int fs_chmod_file_0664(const char *filepath)
{
    if (chmod(filepath, 0664) != 0) {
        fprintf(stderr, "Failed to set permissions for %s: %s\n", filepath, strerror(errno));
        return -1;
    }
    return 0;
}