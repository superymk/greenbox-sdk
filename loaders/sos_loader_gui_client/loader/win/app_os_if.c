// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <appddk.h>
// #include <stdio.h>
// #include <fcntl.h>	// for open
// #include <unistd.h> // for close
// #include <errno.h>	// for errno
// // #include <stdlib.h> // for malloc, free
// #include <string.h> /* memset */
// #include <sys/types.h>
// #include <sys/sysinfo.h>
// #include <sys/time.h>

#include <stdio.h>

#include <windows.h>
#include <memoryapi.h>

#include <loader_sa_intfs.h>

#include "../gui/util_log.h"

#define IOCTL_SECAGENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _IO_OPERATION
{
    HANDLE hDevice;
    PVOID pInputBuffer;
    DWORD dwInputBufferLength;
    PVOID pOutputBuffer;
    DWORD dwOutputBufferLength;
    OVERLAPPED overlapped;
} IO_OPERATION, *PIO_OPERATION;

static vaddr_t app_mm_alloc(uint32_t size)
{
    void *result = NULL;

    result = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (result)
        memset(result, 0, size);

    return (vaddr_t)result;
}

static void app_mm_free(vaddr_t vaddr)
{
    bool status = false;

    status = VirtualFree((void *)vaddr, 0, MEM_RELEASE);
    if (!status)
    	FileUtils_Log("Free memory error! Error:#%d\n", (int)GetLastError());
}

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
    hDevice = CreateFile(L"\\\\.\\SecAgent",
                         (GENERIC_READ | GENERIC_WRITE),
                         (FILE_SHARE_READ | FILE_SHARE_WRITE),
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        FileUtils_Log("<os_ioctl> Access SecAgent Error (Error: #%d)\n", (err_status = (int)GetLastError()));
        goto out;
    }

    // Enable concurrent ioctl
    pIoOperation = (PIO_OPERATION)app_mm_alloc(sizeof(IO_OPERATION));
    if (!pIoOperation)
    {
        FileUtils_Log("<os_ioctl> Failed to create <pIoOperation>!\n");
        goto out;
    }

    resp = (struct tz_cmd_packet *)app_mm_alloc(sizeof(struct tz_cmd_packet));
    if (!resp)
    {
        FileUtils_Log("<os_ioctl> Failed to create <resp>!\n");
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
        FileUtils_Log("<os_ioctl> Failed to create event\n");
        goto out;
    }

    pIo = CreateThreadpoolIo(hDevice, IoCompletionRoutine, pIoOperation, NULL);
    if (!pIo)
    {
        FileUtils_Log("<os_ioctl> Failed to create IO object\n");
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
        err_status = (int)GetLastError();

        if (err_status == ERROR_IO_PENDING) {
            // EXPECTED for overlapped I/O. Not an error.
            status = true;
        } else {
            // Real failure: must cancel the TP I/O "started" accounting.
            CancelThreadpoolIo(pIo);
            FileUtils_Log("<os_ioctl> DeviceIoControl failed (Error: #%d)\n", err_status);
            status = false;
            goto out_wait; // still go to wait path carefully (see below)
        }
    }
    else
    {
        // Completed synchronously (still may get a TP callback; depends on completion mechanism)
        status = true;
    }

out_wait:
    // Wait for all IO operations to complete
    WaitForThreadpoolIoCallbacks(pIo, false);

    // At this point I/O is complete (either callback ran, or synchronous completion).
    // Validate result if you want:
    if (status) 
    {
        DWORD bytes = 0;
        BOOL ok = GetOverlappedResult(hDevice, &pIoOperation->overlapped, &bytes, FALSE);
        if (!ok) 
        {
            err_status = (int)GetLastError();
            FileUtils_Log("<os_ioctl> GetOverlappedResult failed (Error: #%d)\n", err_status);
            status = false;
        } else 
        {
            // Copy output back to caller-visible cmd
            if (bytes >= sizeof(struct tz_cmd_packet)) {
                memcpy(cmd, resp, sizeof(struct tz_cmd_packet));
            } else {
                // driver returned too few bytes
                FileUtils_Log("<os_ioctl> IOCTL returned %lu bytes (< tz_cmd_packet)\n", bytes);
                status = false;
            }
        }
    }

out:
    if (pIo)
        CloseThreadpoolIo(pIo);
    if (pIoOperation)
    {
        if (pIoOperation->overlapped.hEvent)
            CloseHandle(pIoOperation->overlapped.hEvent);

        app_mm_free((vaddr_t)pIoOperation);
    }
    if (resp)
        app_mm_free((vaddr_t)resp);
    CloseHandle(hDevice);
    if (err_no)
        *err_no = err_status;

    // FileUtils_Log("[Superymk] <os_ioctl> Finished cmd:%u\n", cmd->id);
    return status;
}

wchar_t *win_convert_char_to_LPCWSTR(const char *charStr)
{
    if (charStr == NULL)
        return L""; // Return a pointer to a constant empty wide string

    // Determine the required buffer size (including the null terminator)
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, charStr, -1, NULL, 0);
    if (bufferSize == 0)
    {
        // Handle error
        FileUtils_Log("Error in MultiByteToWideChar: %d\n", GetLastError());
        return L"";
    }

    // Allocate memory for the wide-character string
    wchar_t *wideStr = (wchar_t *)malloc(bufferSize * sizeof(wchar_t));
    if (wideStr == NULL)
    {
        FileUtils_Log("Memory allocation failed\n");
        return L"";
    }

    // Perform the conversion
    int result = MultiByteToWideChar(CP_UTF8, 0, charStr, -1, wideStr, bufferSize);
    if (result == 0)
    {
        // Handle error
        FileUtils_Log("Error in MultiByteToWideChar: %d\n", GetLastError());
        free(wideStr);
        return L"";
    }

    return wideStr;
}