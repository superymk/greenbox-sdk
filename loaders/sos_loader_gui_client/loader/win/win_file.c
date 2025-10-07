// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <appddk.h>
#include "../include/loader_os_intfs.h"

#include <stdio.h>
#include <fcntl.h>  // for open
#include <errno.h>  // for errno
// #include <stdlib.h> // for malloc, free
#include <string.h> /* memset */
#include <direct.h>

#include <windows.h>
#include <aclapi.h>
#include <memoryapi.h>

extern wchar_t* win_convert_char_to_LPCWSTR(const char* charStr);

int os_file_lock_create(const char* filepath, ulong_t* out_fd)
{
    HANDLE hFile;
    wchar_t wFilePath[MAX_PATH]; // Make sure the path doesn't exceed MAX_PATH characters

    // Convert char* to wchar_t*
    MultiByteToWideChar(CP_UTF8, 0, filepath, -1, wFilePath, MAX_PATH);

    // Open or create the file exclusively with read/write permissions
    hFile = CreateFile(
        wFilePath,                // File name
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
    if (!WriteFile(hFile, text, (DWORD)(strlen(text)), &bytesWritten, NULL)) {
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
    char tempPath[MAX_PATH];
    strncpy(tempPath, filepath, MAX_PATH - 1);
    tempPath[MAX_PATH - 1] = '\0'; // Ensure null termination

    // Find the position of the last separator to avoid creating a directory for a file
    char *last_sep = NULL;
    for (char *p = tempPath; *p; ++p)
    {
        if (*p == '\\' || *p == '/')
        {
            last_sep = p;
        }
    }

    // If there is a last separator, temporarily terminate the string there
    if (last_sep)
    {
        char saved_char = *(last_sep + 1);
        *(last_sep + 1) = '\0';

        // Now create directories up to this point
        // Start from the beginning
        for (char *p = tempPath; *p; ++p)
        {
            if (*p == '\\' || *p == '/')
            {
                // Temporarily replace the separator with null
                char old = *p;
                *p = '\0';

                // Check if the current path is just a drive letter (e.g., "D:")
                if (!(strlen(tempPath) == 2 && isalpha(tempPath[0]) && tempPath[1] == ':'))
                {
                    // Attempt to create the directory
                    if (_mkdir(tempPath) != 0 && errno != EEXIST)
                    {
                        printf("Error creating directory %s: %s\n", tempPath, strerror(errno));
                        // Restore the saved character
                        *(last_sep + 1) = saved_char;
                        return -1;
                    }
                }

                // Restore the separator
                *p = old;
            }
        }

        // Restore the saved character
        *(last_sep + 1) = saved_char;
    }
    else
    {
        // No separators, nothing to create
        return 0;
    }

    return 0;
}

FILE* fopen_normalized_path(const char *filepath, const char* mode)
{
    return fopen(filepath, mode);
}

int fs_read_file(const char *filepath, bool create_if_not_exist, uint8_t **data, size_t *len)
{
    HANDLE hFile;
    wchar_t* wfilepath = NULL;

    wfilepath = win_convert_char_to_LPCWSTR(filepath);
    if (wfilepath == L"")
    {
        return -1;
    }

    if(create_if_not_exist)
    {
        hFile = CreateFile(wfilepath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else
    {
        hFile = CreateFile(wfilepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    free(wfilepath);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Error opening file %s: %ld\n", filepath, GetLastError());
        return -1;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        printf("Error getting file size: %ld\n", GetLastError());
        CloseHandle(hFile);
        return -1;
    }

    *data = (uint8_t *)malloc(fileSize.QuadPart);
    if (*data == NULL)
    {
        printf("Memory allocation failed\n");
        CloseHandle(hFile);
        return -1;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, *data, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != fileSize.QuadPart)
    {
        printf("Error reading file: %ld\n", GetLastError());
        free(*data);
        CloseHandle(hFile);
        return -1;
    }

    *len = (size_t)fileSize.QuadPart;
    CloseHandle(hFile);
    return 0;
}

int fs_write_file(const char *filepath, const uint8_t *data, size_t len)
{
    wchar_t* wfilepath = NULL;

    // Create directories if needed
    if (fs_create_directories(filepath) != 0)
    {
        return -1;
    }

    wfilepath = win_convert_char_to_LPCWSTR(filepath);
    if (wfilepath == L"")
    {
        return -1;
    }

    HANDLE hFile = CreateFile(wfilepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    free(wfilepath);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Error creating file %s: %ld\n", filepath, GetLastError());
        return -1;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, data, (DWORD)len, &bytesWritten, NULL) || bytesWritten != len)
    {
        printf("Error writing to file: %ld\n", GetLastError());
        CloseHandle(hFile);
        return -1;
    }

    CloseHandle(hFile);
    return 0;
}

int fs_delete_file(const char *filepath)
{
    // Attempt to delete the file
    if (!DeleteFileA(filepath))
    {
        DWORD error_code = GetLastError();

        // Handle specific errors if needed
        switch (error_code)
        {
            case ERROR_FILE_NOT_FOUND:
                // File not exist
                return 0;
            case ERROR_ACCESS_DENIED:
                printf("Error deleting file %s: Access denied.\n", filepath);
                break;
            case ERROR_SHARING_VIOLATION:
                printf("Error deleting file %s: Sharing violation.\n", filepath);
                break;
            case ERROR_PATH_NOT_FOUND:
                printf("Error deleting file %s: Path not found.\n", filepath);
                break;
            default:
                printf("Error deleting file %s: %ld\n", filepath, error_code);
                break;
        }

        return -1;
    }

    // Deletion successful
    return 0;
}

int fs_chmod_file_0664(const char *filepath)
{
    // Define the security descriptor and ACL
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pACL = NULL;

    // Define the explicit access structure
    EXPLICIT_ACCESSA ea[2];
    ZeroMemory(&ea, sizeof(ea));

    // Grant full access to the owner
    ea[0].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea[0].Trustee.ptstrName = "CURRENT_USER";

    // Grant read access to everyone
    ea[1].grfAccessPermissions = GENERIC_READ;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName = "EVERYONE";

    // Create a new ACL
    if (SetEntriesInAclA(2, ea, NULL, &pACL) != ERROR_SUCCESS)
    {
        fprintf(stderr, "Failed to create ACL\n");
        return -1;
    }

    // Create a security descriptor
    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSD == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for security descriptor\n");
        LocalFree(pACL);
        return -1;
    }

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        fprintf(stderr, "Failed to initialize security descriptor\n");
        LocalFree(pACL);
        LocalFree(pSD);
        return -1;
    }

    // Set the DACL in the security descriptor
    if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
    {
        fprintf(stderr, "Failed to set DACL in security descriptor\n");
        LocalFree(pACL);
        LocalFree(pSD);
        return -1;
    }

    // Apply the security descriptor to the file
    if (!SetFileSecurityA(filepath, DACL_SECURITY_INFORMATION, pSD))
    {
        fprintf(stderr, "Failed to set file security: %lu\n", GetLastError());
        LocalFree(pACL);
        LocalFree(pSD);
        return -1;
    }

    // Cleanup
    LocalFree(pACL);
    LocalFree(pSD);

    return 0;
}