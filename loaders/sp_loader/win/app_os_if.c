// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
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
#include <dbghelp.h>
#include <memoryapi.h>
#include <psapi.h>

#include <process.h>

#include <intrin.h>

#include "../hash/hash.h"

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
#define bzero(b, len) (memset((b), '\0', (len)), (void)0)

#ifdef __MSYS2__
#define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ? 0 : errno)
#endif

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

// static void handler(int signal, siginfo_t *si, void *arg)
// {
// 	// void *array[30];
//   	// size_t size;

//     APP_ERR("Caught segfault at address %p\n", si->si_addr);
// 	// get void*'s for all entries on the stack
// 	// size = backtrace(array, 30);

// 	// print out all the frames to stderr
// 	fprintf(stderr, "Error: signal %d:\n", signal);
// 	// backtrace_symbols_fd(array, size, STDERR_FILENO);

// 	// Call SecProcess Loader's exception handler
// 	sp_loader_on_exception();
// }

static LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS *ExceptionInfo)
{
    APP_ERR("Caught Exception %lu\n", (uint64_t)ExceptionInfo->ExceptionRecord->ExceptionCode);

    // Call SecProcess Loader's exception handler
    sp_loader_on_exception();

    return EXCEPTION_EXECUTE_HANDLER;
}

static DWORD_PTR _GetCorrectProcessAffinityMask()
{
    // Get the system information.
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    // Return the processor affinity mask.
    return systemInfo.dwActiveProcessorMask;
}

static BOOL _SetAllNonCurrentProcessAffinityMask(DWORD_PTR dwProcessAffinityMask)
{
    HANDLE hProcess;
    BOOL bResult;
    DWORD sosloader_proc_id;

    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    sosloader_proc_id = GetCurrentProcessId();
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return FALSE;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.
    for (i = 0; i < cProcesses; i++)
    {
        DWORD dwProcessID = aProcesses[i];
        if (dwProcessID != 0 && dwProcessID != sosloader_proc_id)
        {
            hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessID);
            if (hProcess != NULL)
            {
                bResult = SetProcessAffinityMask(hProcess, dwProcessAffinityMask);
                if (!bResult)
                {
                    CloseHandle(hProcess);
                    break;
                }

                CloseHandle(hProcess);
            }
        }
    }

    return TRUE;
}

/// @brief Utility function to read the entire file into memory
static int read_file(const char *filepath, uint8_t **data, size_t *len)
{
    HANDLE hFile = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        APP_ERR("Error opening file %s: %d\n", filepath, GetLastError());
        return -1;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        APP_ERR("Error getting file size: %d\n", GetLastError());
        CloseHandle(hFile);
        return -1;
    }

    *data = (uint8_t *)malloc(fileSize.QuadPart);
    if (*data == NULL)
    {
        APP_ERR("Memory allocation failed\n");
        CloseHandle(hFile);
        return -1;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, *data, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != fileSize.QuadPart)
    {
        APP_ERR("Error reading file: %d\n", GetLastError());
        free(*data);
        CloseHandle(hFile);
        return -1;
    }

    *len = (size_t)fileSize.QuadPart;
    CloseHandle(hFile);
    return 0;
}

/************ Public Functions **********/
void sp_loader_install_exception_handler(void)
{
    SetUnhandledExceptionFilter(windows_exception_handler);
}

void msleep(uint32_t ms)
{
    Sleep(ms);
}

void os_atomic_increase(uint32_t *var)
{
    InterlockedExchangeAdd((volatile int *)var, 1);
}

void os_atomic_decrease(uint32_t *var)
{
    InterlockedExchangeAdd((volatile int *)var, -1);
}

bool app_set_cpu_affinity(uint32_t cpu)
{
    DWORD_PTR affinityMask = 1 << cpu;
    DWORD_PTR ret = 0;

    ret = SetThreadAffinityMask(GetCurrentThread(), affinityMask);
    if (!ret)
    {
        APP_ERR("Set CPU Affinity Error (Error: #%d)! affinityMask:0x%lX\n",
                (ret = (int)GetLastError()), affinityMask);
        return false;
    }

    return true;
}

bool kick_all_other_processes_from_core(uint32_t cpu)
{
    DWORD_PTR affinityMask = _GetCorrectProcessAffinityMask() & ~(1 << cpu);

    return (_SetAllNonCurrentProcessAffinityMask(affinityMask) == TRUE) ? true : false;
}

bool thread_set_high_priority(void)
{
    DWORD_PTR ret = 0;

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
    {
        APP_ERR("Set Thread Priority to High Error (Error: #%d)!\n",
                (ret = (int)GetLastError()));
        return false;
    }

    return true;
}

app_pid_t app_get_current_procid(void)
{
    return (app_pid_t)GetCurrentProcessId();
}

/// @brief Return the number of processors
/// @param
/// @return
uint32_t app_cpus_get_procs_num(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    return (uint32_t)sysinfo.dwNumberOfProcessors;
}

void press_continue(const char *prompt)
{
    APP_ERR("%s", prompt);

    if (isatty(fileno(stdin)))
    {
        // Interactive console: your original loop
        for (;;)
        {
            int ch = getchar();
            if (ch == '\n' || ch == EOF)
                break;

            // Put a sleep in order to yield CPU
            // [NOTE] sched_yield() does not reduce CPU usage somehow...
            msleep(100);
        }
        return;
    }
    else
    {
        // Non-interactive (QProcess pipe or no console):
        // Block until the parent writes something OR closes the pipe.
        // Use a single blocking getchar(); we don't need a loop.
        (void)getchar(); // returns when parent writes "\r\n" or closes write-end
    }
}

static HANDLE _eventfd_from_env(void)
{
    char name[256] = {0};
    DWORD got = GetEnvironmentVariableA("GB_APPLOADER_EXIT_FD", name, sizeof(name));
    if (got == 0 || got >= sizeof(name))
        return NULL;
    return OpenEventA(SYNCHRONIZE, FALSE, name);
}

static void _wait_for_exit_noninteractive(void)
{
    HANDLE h = _eventfd_from_env();
    if (!h)
    {
        // Fallback: just sleep forever until process is killed
        for (;;)
            msleep(100);
    }
    WaitForSingleObject(h, INFINITE);
}

void app_wait_for_exit(void)
{
    if (isatty(fileno(stdin)))
    {
        press_continue("Press to exit SecApp");
    }
    else
    {
        _wait_for_exit_noninteractive();
    }
}

#define IOCTL_SECAGENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)

void sp_loader_os_init()
{
}

void sp_loader_os_fini()
{
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
    hDevice = CreateFile("\\\\.\\SecAgent",
                         (GENERIC_READ | GENERIC_WRITE),
                         (FILE_SHARE_READ | FILE_SHARE_WRITE),
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        APP_ERR("<os_ioctl> Access SecAgent Error (Error: #%d)\n",
                (err_status = (int)GetLastError()));
        goto out;
    }

    // Enable concurrent ioctl
    pIoOperation = (PIO_OPERATION)app_mm_alloc(sizeof(IO_OPERATION));
    if (!pIoOperation)
    {
        APP_ERR("<os_ioctl> Failed to create <pIoOperation>!\n");
        goto out;
    }

    resp = (struct tz_cmd_packet *)app_mm_alloc(sizeof(struct tz_cmd_packet));
    if (!resp)
    {
        APP_ERR("<os_ioctl> Failed to create <resp>!\n");
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
        APP_ERR("<os_ioctl> Failed to create event\n");
        goto out;
    }

    pIo = CreateThreadpoolIo(hDevice, IoCompletionRoutine, pIoOperation, NULL);
    if (!pIo)
    {
        APP_ERR("<os_ioctl> Failed to create IO object\n");
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
            APP_ERR("<os_ioctl> DeviceIoControl failed (Error: #%d)\n", err_status);
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
            APP_ERR("<os_ioctl> GetOverlappedResult failed (Error: #%d)\n", err_status);
            status = false;
        } else 
        {
            // Copy output back to caller-visible cmd
            if (bytes >= sizeof(struct tz_cmd_packet)) {
                memcpy(cmd, resp, sizeof(struct tz_cmd_packet));
            } else {
                // driver returned too few bytes
                APP_ERR("<os_ioctl> IOCTL returned %lu bytes (< tz_cmd_packet)\n", bytes);
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

    // APP_ERR("[Superymk] <os_ioctl> Finished cmd:%u\n", cmd->id);
    return status;
}

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
            APP_ERR("<app_lock_mem> Error to read current working set limit!\n");
            return -1;
        }

        min_ws += len;
        max_ws += len;
        status = SetProcessWorkingSetSize(GetCurrentProcess(), min_ws, max_ws);
        if (!status)
        {
            APP_ERR("<app_lock_mem> Error to increase working set limit to min_ws:0x%llX, max_ws:0x%llX!\n", min_ws, max_ws);
            return -2;
        }

        // try locking again
        if (mlock(ptr, len))
        {
            APP_ERR("<app_lock_mem> Error to lock memory %p with size 0x%lx again! Error: #%d\n", TO_PTR(ptr), len, (int)GetLastError());
            return -3;
        }

        app_touch_mem(ptr, len);
        return 0;
    }

    APP_ERR("<app_lock_mem> Failed to lock memory 0x%lX with size %u\n", (vaddr_t)ptr, len);
    return -4;
}

int app_unlock_mem(const void *addr, size_t len)
{
    int status = munlock(addr, len);
    if (status)
        APP_ERR("Unlock memory %p with size 0x%lx error! Error: #%d\n", TO_PTR(addr), len, (int)GetLastError());

    return status;
}

vaddr_t app_mm_alloc(uint32_t size)
{
    void *result = NULL;

    result = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (result)
        bzero(result, size);

    return (vaddr_t)result;
}

__attribute__((unused)) static void _print_backtrace(void)
{
    // Initialize variables
    void *stack[30];
    unsigned short frames;
    SYMBOL_INFO *symbol;
    HANDLE process;

    // Get the current process handle
    process = GetCurrentProcess();

    // Initialize the symbol handler for the process
    if (!SymInitialize(process, NULL, TRUE))
    {
        APP_ERR("SymInitialize failed with error %u\n", GetLastError());
        return;
    }

    // Capture the stack backtrace
    frames = CaptureStackBackTrace(0, 30, stack, NULL);
    if (frames == 0)
    {
        APP_ERR("CaptureStackBackTrace failed\n");
        SymCleanup(process);
        return;
    }

    // Allocate memory for SYMBOL_INFO structure
    symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    if (!symbol)
    {
        APP_ERR("Memory allocation for SYMBOL_INFO failed\n");
        SymCleanup(process);
        return;
    }

    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    // Iterate over captured frames and print symbol information
    for (int i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        if (SymFromAddr(process, address, 0, symbol))
        {
            APP_ERR("%i: %s - 0x%0llX\n", i, symbol->Name, symbol->Address);
        }
        else
        {
            APP_ERR("%i: [Symbol not found] - 0x%0llX\n", i, address);
        }
    }

    // Clean up
    free(symbol);
    SymCleanup(process);
}

void app_mm_free(vaddr_t vaddr)
{
    bool status = false;

    status = VirtualFree((void *)vaddr, 0, MEM_RELEASE);
    if (!status)
    {
        APP_ERR("Free memory error! Error:#%d\n", (int)GetLastError());
    }
}

// static vaddr_t app_mm_alloc_aligned(uint32_t size, uint32_t alignment, struct mem_alignment_info *info)
// {
// 	void *result = NULL;
// 	uint32_t mem_size = ALIGN(size, alignment);

// 	if (!info)
// 		return INVALID_VADDR;

// 	result = VirtualAlloc(NULL, mem_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
// 	if (result)
// 		bzero(result, mem_size);

// 	info->_buf_start_vaddr = (vaddr_t)result;
// 	info->_buf_size = mem_size;

// 	return (vaddr_t)ALIGN((vaddr_t)result, alignment);
// }

// static void app_mm_free_aligned(vaddr_t vaddr, struct mem_alignment_info *info)
// {
// 	bool status = false;

// 	(void)vaddr;
// 	if (!info)
// 		return;

// 	status = VirtualFree((void *)info->_buf_start_vaddr, 0, MEM_RELEASE);
// 	if (!status)
// 		APP_ERR("Free aligned memory error! Error:#%d\n", (int)GetLastError());
// }

static uint32_t secagent_alloc_and_lock_pages_for_sdom(uint32_t num_pages, vaddr_t *out_vaddr)
{
    OSSTATUS sa_status = OS_SUCCESS;
    bool status = false;
    struct tz_cmd_packet cmd;
    int ret = 0;

    if (!num_pages || !out_vaddr)
        return 1;

    cmd.id = CMD_ALLOC_AND_LOCK_PAGES_FOR_SDOM;
    cmd.arg1 = num_pages;
    status = os_ioctl(&cmd, &ret);
    if (!status)
    {
        APP_ERR("<secagent_alloc_and_lock_pages_for_sdom> error with code %d!\n", ret);
        return 1;
    }

    sa_status = cmd.status;
    if (SA_FAIL(sa_status))
    {
        APP_ERR("<secagent_alloc_and_lock_pages_for_sdom> SecAgent error: %d!\n", sa_status);
        return 1;
    }

    *out_vaddr = cmd.arg1;
    return 0;
}

static uint32_t secagent_free_and_unlock_pages_for_sdom(vaddr_t vaddr)
{
    OSSTATUS sa_status = OS_SUCCESS;
    bool status = false;
    struct tz_cmd_packet cmd;
    int ret = 0;

    if (!vaddr)
        return 1;

    cmd.id = CMD_FREE_AND_UNLOCK_PAGES_FOR_SDOM;
    cmd.arg1 = vaddr;
    status = os_ioctl(&cmd, &ret);
    if (!status)
    {
        APP_ERR("<secagent_free_and_unlock_pages_for_sdom> error with code %d!\n", ret);
        return 1;
    }

    sa_status = cmd.status;
    if (SA_FAIL(sa_status))
    {
        APP_ERR("<secagent_free_and_unlock_pages_for_sdom> SecAgent error: %d!\n", sa_status);
        return 1;
    }

    return 0;
}

vaddr_t app_mm_alloc_and_lock_4Kpages_for_sdom(uint32_t num_pages, struct mem_alignment_info *info)
{
    uint32_t ret = 0;
    vaddr_t vaddr = INVALID_VADDR;

    if (!info)
        return INVALID_VADDR;

    ret = secagent_alloc_and_lock_pages_for_sdom(num_pages, &vaddr);
    if (ret)
        return INVALID_VADDR;

    // Zero the memory
    memset((void *)vaddr, 0, num_pages << PAGE_SHIFT_4K);

    info->_buf_start_vaddr = vaddr;
    info->_buf_size = num_pages << PAGE_SHIFT_4K;

    return vaddr;
}

void app_mm_free_and_unlock_4Kpages_for_sdom(struct mem_alignment_info *info)
{
    uint32_t ret = 0;

    if (!info)
        return;

    // Zero the memory
    memset((void *)info->_buf_start_vaddr, 0, info->_buf_size);

    ret = secagent_free_and_unlock_pages_for_sdom(info->_buf_start_vaddr);
    if (ret)
        return;
}

void app_mm_memcpy(void *dst, const void *src, size_t size)
{
    memcpy(dst, src, size);
}

void *app_mm_memset(void *m, int c, size_t n)
{
    char *s = (char *)m;

    while (n--)
        *s++ = (char)c;

    return m;
}

bool file_read(const char *filename, void *buf, uint32_t offset, size_t size)
{
    bool result = true;
    FILE *f;
    int n;

    // Sanity checks
    if (!filename || !buf)
        return false;

    f = fopen_normalized_path(filename, "rb");
    if (f)
    {
        fseek(f, offset, SEEK_SET);
        n = fread(buf, size, 1, f);
        if (n < 1)
        {
            APP_ERR("<file_read> Insufficient content to read\n");
            result = false;
        }
    }
    else
    {
        APP_ERR("<file_read> Read file error\n");
        result = false;
    }
    fclose(f);

    return result;
}

bool file_get_size(const char *filename, size_t *out_sz)
{
    int ret = 0;
    ret = fs_file_get_size(filename, out_sz);
    if (ret)
        return false;

    // On success
    return true;
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
        if (read_file(exe_filepath, &file_data, &file_size) != 0)
        {
            return -1;
        }

        uint8_t file_hash[SHA256_DIGEST_LENGTH];
        if (sha2_256_mem(file_data, file_size, file_hash) != 0)
        {
            free(file_data);
            APP_ERR("Hash computation failed\n");
            return -1;
        }

        free(file_data);

        if (memcmp(file_hash, exe_file_hash->sha2_256_digest, SHA256_DIGEST_LENGTH) != 0)
        {
            APP_ERR("Executable file hash does not match (%s)\n", exe_filepath);
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
    result = spawnv(_P_WAIT, exe_filepath, (const char *const *)argv);

    // Check process result
    if (result == -1)
    {
        APP_ERR("_spawnv");
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

ulong_t os_execute_program_nowait(
    bool ignore_child_status,
    bool hide_window,
    const char *exe_filepath,
    const union sha_digest *exe_file_hash,
    const char *exe_name,
    const char *first_arg,
    ...)
{
    if (!exe_filepath || !exe_name)
        return (ulong_t)-1;

    // --- optional hash check ---
    if (exe_file_hash)
    {
        uint8_t *file_data;
        size_t file_size;
        if (read_file(exe_filepath, &file_data, &file_size) != 0)
            return (ulong_t)-1;

        uint8_t file_hash[SHA256_DIGEST_LENGTH];
        if (sha2_256_mem(file_data, file_size, file_hash) != 0)
        {
            free(file_data);
            fprintf(stderr, "Hash computation failed\n");
            return (ulong_t)-1;
        }
        free(file_data);

        if (memcmp(file_hash,
                   exe_file_hash->sha2_256_digest,
                   SHA256_DIGEST_LENGTH) != 0)
        {
            fprintf(stderr, "Executable file hash does not match\n");
            return (ulong_t)-1;
        }
    }

    // --- build argv list ---
    const int MAX_ARGS = 128;
    char *argv[MAX_ARGS];
    int argc = 0;

    va_list ap;
    va_start(ap, first_arg);

    // argv[0] = full path to exe
    argv[argc++] = strdup(exe_filepath);
    // argv[1] = first user arg
    argv[argc++] = strdup(first_arg);

    // remaining varargs until NULL or limit
    const char *arg;
    while ((arg = va_arg(ap, const char *)) != NULL && argc < MAX_ARGS - 1)
    {
        argv[argc++] = strdup(arg);
    }
    va_end(ap);
    argv[argc] = NULL; // null-terminate

    // --- build a single command-line string ---
    // (CreateProcess wants one C string of all args)
    size_t cmd_len = 0;
    for (int i = 0; i < argc; ++i)
    {
        // +3 for two quotes and a space (or NUL at end)
        cmd_len += strlen(argv[i]) + 3;
    }

    char *cmd_line = malloc(cmd_len + 1);
    if (!cmd_line)
    {
        APP_ERR("malloc(cmd_line)");
        for (int i = 0; i < argc; ++i)
            free(argv[i]);
        return (ulong_t)-1;
    }
    cmd_line[0] = '\0';

    for (int i = 0; i < argc; ++i)
    {
        strcat(cmd_line, "\"");
        strcat(cmd_line, argv[i]);
        strcat(cmd_line, "\"");
        if (i < argc - 1)
            strcat(cmd_line, " ");
    }

    // --- set up STARTUPINFO to hide window if requested ---
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (hide_window)
    {
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    DWORD creationFlags = hide_window
                              ? CREATE_NO_WINDOW
                              : 0;

    BOOL ok = CreateProcessA(
        exe_filepath,  // lpApplicationName
        cmd_line,      // lpCommandLine (w/ quotes)
        NULL,          // lpProcessAttributes
        NULL,          // lpThreadAttributes
        FALSE,         // bInheritHandles
        creationFlags, // dwCreationFlags
        NULL,          // lpEnvironment
        NULL,          // lpCurrentDirectory
        &si,
        &pi);

    free(cmd_line);

    intptr_t hProcess = -1;
    if (!ok)
    {
        DWORD err = GetLastError();
        fprintf(stderr, "CreateProcessA failed (error %u)\n", err);
    }
    else
    {
        hProcess = (intptr_t)pi.hProcess;
        CloseHandle(pi.hThread); // we only need the process handle
    }

    // clean up our strdup’ed args
    for (int i = 0; i < argc; ++i)
    {
        free(argv[i]);
    }

    // if caller doesn’t care about failure, map errors → 0
    if (ignore_child_status && hProcess == -1)
        return 0;

    return (ulong_t)hProcess;
}

// -----------------------------------------------------------------------------
// os_program_kill
//    - given the CRT process‐handle from os_execute_program_nowait,
//      calls TerminateProcess + closes the handle.
// returns: 0 on success, -1 on failure.
// -----------------------------------------------------------------------------
int os_program_kill(ulong_t process_handle)
{
    if (process_handle == (intptr_t)-1 || process_handle == 0)
        return 1;

    HANDLE hProc = (HANDLE)process_handle;
    if (!TerminateProcess(hProc, /*exit code*/ 1))
    {
        fprintf(stderr, "TerminateProcess failed: %u\n", GetLastError());
        CloseHandle(hProc);
        return 1;
    }

    CloseHandle(hProc);
    return 0;
}