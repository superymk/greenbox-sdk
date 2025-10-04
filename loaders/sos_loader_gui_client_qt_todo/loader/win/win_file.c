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
// #include <appddk.h>
#include "../include/loader_os_intfs.h"

#include <windows.h>
#include <stdio.h>

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