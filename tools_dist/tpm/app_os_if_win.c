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

#ifdef __WINDOWS__

#include <toolsddk.h>
#include <stdio.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <errno.h>  // for errno
// #include <stdlib.h> // for malloc, free
#include <string.h> /* memset */
#include <sys/types.h>
// #include <sys/sysinfo.h>
// #include <sys/time.h>
#include <windows.h>
#include <aclapi.h>
#include <memoryapi.h>
#include <psapi.h>

#include "utils.h"
#include "./hash/hash.h"

typedef struct _IO_OPERATION
{
    HANDLE hDevice;
    PVOID pInputBuffer;
    DWORD dwInputBufferLength;
    PVOID pOutputBuffer;
    DWORD dwOutputBufferLength;
    OVERLAPPED overlapped;
} IO_OPERATION, *PIO_OPERATION;

/************ Utility Functions **********/
static int mlock(const void *addr, size_t len)
{
    bool result = false;

    result = VirtualLock((void *)addr, len);
    if (!result)
        return -1;

    return 0;
}

static int munlock(const void *addr, size_t len)
{
    bool result = false;

    result = VirtualUnlock((void *)addr, len);
    if (!result)
        return -1;

    return 0;
}

/************ Public Functions **********/
int app_lock_mem(void *ptr, size_t len)
{
    ptr = (void *)SIZE_ALIGN_4K((vaddr_t)ptr);
    len = SIZE_ROUNDUP_4K(len);

    if (!mlock(ptr, len))
    {
        app_touch_mem(ptr, len);
        return 0;
    }

    if (GetLastError() == ERROR_WORKING_SET_QUOTA)
    {
        // try increasing working set limits
        bool status = false;
        SIZE_T min_ws = 0;
        SIZE_T max_ws = 0;

        status = GetProcessWorkingSetSize(GetCurrentProcess(), &min_ws, &max_ws);
        if (!status)
        {
            printf("<app_lock_mem> Error to read current working set limit!\n");
            return -1;
        }

        min_ws += len;
        max_ws += len;
        status = SetProcessWorkingSetSize(GetCurrentProcess(), min_ws, max_ws);
        if (!status)
        {
            printf("<app_lock_mem> Error to increase working set limit to min_ws:0x%llX, max_ws:0x%llX!\n", min_ws, max_ws);
            return -2;
        }

        // try locking again
        if (mlock(ptr, len))
        {
            printf("<app_lock_mem> Error to lock memory %p with size 0x%llx again! Error: #%d\n", TO_PTR(ptr), len, (int)GetLastError());
            return -3;
        }

        app_touch_mem(ptr, len);
        return 0;
    }

    printf("<app_lock_mem> Failed to lock memory 0x%llX with size %llu\n", (vaddr_t)ptr, len);
    return -4;
}

int app_unlock_mem(const void *addr, size_t len)
{
    int status = munlock(addr, len);
    if (status)
        printf("Unlock memory %p with size 0x%llx error! Error: #%d\n", TO_PTR(addr), len, (int)GetLastError());

    return status;
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
                    if (mkdir(tempPath) != 0 && errno != EEXIST)
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


int fs_read_file(const char *filepath, bool create_if_not_exist, uint8_t **data, size_t *len)
{
    HANDLE hFile;

    if(create_if_not_exist)
    {
        hFile = CreateFile(filepath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else
    {
        hFile = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

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
    // Create directories if needed
    if (fs_create_directories(filepath) != 0)
    {
        return -1;
    }

    HANDLE hFile = CreateFile(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

#define IOCTL_SECAGENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)

static VOID CALLBACK IoCompletionRoutine(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PVOID Overlapped,
    ULONG IoResult,
    ULONG_PTR NumberOfBytesTransferred,
    PTP_IO Io)
{
    PIO_OPERATION pIoOperation = (PIO_OPERATION)Context;
    struct tz_cmd_packet *cmd = (struct tz_cmd_packet *)pIoOperation->pInputBuffer;
    struct tz_cmd_packet *resp = (struct tz_cmd_packet *)pIoOperation->pOutputBuffer;

    if (IoResult == ERROR_SUCCESS)
    {
        // process results in pIoOperation->pBuffer
        // Overwrite input <cmd> with the returned response
        memcpy(cmd, resp, sizeof(struct tz_cmd_packet));
    }
    else
    {
        // handle error
        cmd->status = E_INTERNAL_ERROR;
    }
}

bool os_ioctl(struct tz_cmd_packet *cmd, int *err_no)
{
    PIO_OPERATION pIoOperation = NULL;
    PTP_IO pIo = NULL;
    HANDLE hDevice;
    struct tz_cmd_packet *resp = NULL;
    DWORD ret_data_sz;

    /* call kernel module to allocate a physically-contiguous
     * non-cacheable physical memory pool */
    int ret = 0, err_status = 0;
    bool status = false;

    // Open Device
    hDevice = CreateFile("\\\\.\\SecAgent",
                         (GENERIC_READ | GENERIC_WRITE),
                         (FILE_SHARE_READ | FILE_SHARE_WRITE),
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("<os_ioctl> Access SecAgent Error (Error: #%d)\n",
                (err_status = (int)GetLastError()));
        goto out;
    }

    // Enable concurrent ioctl
    pIoOperation = (PIO_OPERATION)malloc(sizeof(IO_OPERATION));
    if (!pIoOperation)
    {
        printf("<os_ioctl> Failed to create <pIoOperation>!\n");
        goto out;
    }

    resp = (struct tz_cmd_packet *)malloc(sizeof(struct tz_cmd_packet));
    if (!resp)
    {
        printf("<os_ioctl> Failed to create <resp>!\n");
        goto out;
    }

    pIoOperation->hDevice = hDevice;
    pIoOperation->pInputBuffer = cmd;
    pIoOperation->dwInputBufferLength = sizeof(struct tz_cmd_packet);
    pIoOperation->pOutputBuffer = resp;
    pIoOperation->dwOutputBufferLength = sizeof(struct tz_cmd_packet);

    memset(&pIoOperation->overlapped, 0, sizeof(OVERLAPPED));
    pIoOperation->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (pIoOperation->overlapped.hEvent == NULL)
    {
        printf("<os_ioctl> Failed to create event\n");
        goto out;
    }

    pIo = CreateThreadpoolIo(hDevice, &IoCompletionRoutine, pIoOperation, NULL);
    if (!pIo)
    {
        printf("<os_ioctl> Failed to create IO object\n");
        goto out;
    }

    StartThreadpoolIo(pIo);

    // Send IOCTL
    ret = DeviceIoControl(hDevice,
                          IOCTL_SECAGENT,
                          pIoOperation->pInputBuffer,
                          pIoOperation->dwInputBufferLength,
                          pIoOperation->pOutputBuffer,
                          pIoOperation->dwOutputBufferLength,
                          &ret_data_sz,
                          &pIoOperation->overlapped);
    if (!ret)
    {
        //-f--> Oops...
        printf("<os_ioctl> Send IOCTL to SecAgent Error (Error: #%d)\n",
                (err_status = GetLastError()));

        status = false;
    }
    else
        status = true;

    // Wait for all IO operations to complete
    WaitForThreadpoolIoCallbacks(pIo, false);

out:
    if (pIo)
        CloseThreadpoolIo(pIo);
    if (pIoOperation)
        free(pIoOperation);
    if (resp)
        free(resp);
    CloseHandle(hDevice);
    if (err_no)
        *err_no = err_status;

    // printf("[Superymk] <os_ioctl> Finished cmd:%u\n", cmd->id);
    return status;
}

int os_execute_program_wait_exit(bool ignore_child_status, const char *exe_filepath, const union sha_digest *exe_file_hash, const char *exe_name, const char *first_arg, ...)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD returnCode = -1;
    BOOL result;

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
            printf("Hash computation failed\n");
            return -1;
        }

        free(file_data);

        if (memcmp(file_hash, exe_file_hash->sha2_256_digest, SHA256_DIGEST_LENGTH) != 0)
        {
            printf("Executable file hash does not match\n");
            return -1;
        }
    }

    // Prepare arguments for _spawnv
    va_list args;
    va_start(args, first_arg);

    // Assuming we have a limit on the number of arguments
    const int MAX_ARGS = 128;
    char *argv[MAX_ARGS];
    int argc = 0;

    // exe_name is the first argument for consistency with Unix convention
    argv[argc++] = strdup(exe_filepath);
    argv[argc++] = strdup(first_arg);

    const char *temp_arg;
    while ((temp_arg = va_arg(args, const char *)) != NULL && argc < MAX_ARGS - 1)
    {
        argv[argc++] = strdup(temp_arg);
    }
    argv[argc] = NULL; // NULL-terminate the array

    // Freeing the argument list
    va_end(args);

    // Initialize the STARTUPINFO structure
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the process
    result = spawnv(_P_WAIT, exe_filepath, (char *const *)argv);

    // Check process result
    if (result == -1)
    {
        perror("_spawnv");
        returnCode = -1;
    }
    else
    {
        returnCode = result;
    }

    // Freeing the duplicated arguments
    for (int i = 0; i < argc; i++)
    {
        free(argv[i]);
    }

    if (ignore_child_status || returnCode == -1)
    {
        return 0;
    }
    else
    {
        return returnCode;
    }
}

int uxmhf_mhv_locate_first_drive(char* out_drive_str)
{
    DWORD drives = GetLogicalDrives();
    if (drives == 0) {
        printf("Failed to get logical drives. Error: %lu\n", GetLastError());
        return -1;
    }

    // Check: params must be valid
    if(!out_drive_str)
        return -1;

    // Iterate through drive letters A: to Z:
    for (char drive = 'A'; drive <= 'Z'; drive++) {
        // Check if the drive is present
        if (drives & (1 << (drive - 'A'))) {
            char rootPath[4] = { drive, ':', '\\', '\0' };
            
            // Optional: Check the drive type (e.g., skip CD-ROM drives without media)
            UINT driveType = GetDriveTypeA(rootPath);
            if (driveType == DRIVE_NO_ROOT_DIR || driveType == DRIVE_UNKNOWN) {
                continue; // Skip invalid drives
            }

            // Construct the full path to the target file
            char filePath[MAX_PATH];
            snprintf(filePath, sizeof(filePath), "%suxmhfmhv.efi", rootPath);

            // Check if the file exists
            DWORD fileAttr = GetFileAttributesA(filePath);
            if (fileAttr != INVALID_FILE_ATTRIBUTES && 
                !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) 
            {
                snprintf(out_drive_str, MAX_PATH, "%s", rootPath);
                return 0; // Exit after finding the first occurrence
            }
        }
    }

    printf("File uxmhfmhv.efi not found on any drive.\n");
    return -1;
}

#endif // __WINDOWS__