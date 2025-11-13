// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#define _GNU_SOURCE
#include <appddk.h>
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
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <pthread.h>

#include <linux/stat.h>
#include <sys/wait.h>
#include <execinfo.h>

// #define __USE_GNU
#include <sched.h>
// #undef __USE_GNU

#include "../hash/hash.h"

#define LOCK_PREFIX "lock;"

static bool _is_exception_handler_nested = false;
static void handler(int signal, siginfo_t *si, void *arg)
{
    void *array[30];
    size_t size;

    if (!_is_exception_handler_nested)
    {
        // Prevent exception handler nesting
        _is_exception_handler_nested = true;

        // get void*'s for all entries on the stack
        size = backtrace(array, 30);

        // print out all the frames to stderr
        fprintf(stderr, "Error: signal %d:\n", signal);
        backtrace_symbols_fd(array, size, STDERR_FILENO);

        // Call SecProcess Loader's exception handler
        sp_loader_on_exception();
    }
}

/// @brief Utility function to read the entire file into memory
static int read_file(const char *filepath, uint8_t **data, size_t *len)
{
    int fd = open(filepath, O_RDONLY);
    if (fd < 0)
    {
        APP_ERR("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0)
    {
        APP_ERR("fstat");
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
        APP_ERR("read");
        free(*data);
        close(fd);
        return -1;
    }

    *len = st.st_size;
    close(fd);
    return 0;
}

void sp_loader_install_exception_handler(void)
{
    struct sigaction sa;

    // Register sig fault handler
    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_NODEFER;
    sa.sa_sigaction = handler;

    sigaction(SIGSEGV, &sa, NULL); /* ignore whether it works or not */

    // Register Ctrl+C handler
    sigaction(SIGINT, &sa, NULL);
}

void os_atomic_increase(uint32_t *var)
{
    __asm __volatile(LOCK_PREFIX "incl %0"
                     : "=m"(*var)
                     : "m"(*var));
}

void os_atomic_decrease(uint32_t *var)
{
    __asm __volatile(LOCK_PREFIX "decl %0"
                     : "=m"(*var)
                     : "m"(*var));
}

void msleep(uint32_t ms)
{
    struct timespec ts;
    uint32_t second = (ms / 1000);

    ms = ms % 1000;

    ts.tv_sec = second;
    ts.tv_nsec = ms * 1000000;
    nanosleep(&ts, NULL);
}

bool app_set_cpu_affinity(uint32_t cpu)
{
    cpu_set_t mask;
    int result = 0;

    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    result = sched_setaffinity(0, sizeof(mask), &mask);

    if (result)
    {
        APP_ERR("Set CPU Affinity Error: %d\n", result);
        return false;
    }

    return true;
}

bool kick_all_other_processes_from_core(uint32_t cpu)
{
    APP_ERR("<kick_all_other_processes_from_core> is unimplemented\n");

    return true;
}

bool thread_set_high_priority(void)
{
    int ret;
    // We'll operate on the currently running thread.
    pthread_t this_thread = pthread_self();
    struct sched_param params;

    // We'll set the priority to the maximum.
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);

    ret = pthread_setschedparam(this_thread, SCHED_FIFO, &params);
    if (ret != 0)
    {
        // Print the error
        APP_ERR("Set Thread Priority to High Error!\n");
        return false;
    }

    return true;
}

app_pid_t app_get_current_procid(void)
{
    return (app_pid_t)getpid();
}

/// @brief Return the number of processors
/// @param
/// @return
uint32_t app_cpus_get_procs_num(void)
{
    return (uint32_t)get_nprocs();
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

static int _eventfd_from_env(void)
{
    const char *s = getenv("GB_APPLOADER_EXIT_FD");
    if (!s)
        return -1;
    return atoi(s); // fd inherited across exec
}

static void _wait_for_exit_noninteractive(void)
{
    int fd = _eventfd_from_env();
    if (fd < 0)
    {
        // Fallback: block forever
        for (;;)
            msleep(100);
    }

    char buf[1];
    ssize_t n = read(fd, buf, 1); // blocks until parent writes 1 byte or closes
    (void)n;
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
void sp_loader_os_init()
{
}

void sp_loader_os_fini()
{
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
        APP_ERR("Cannot Open Device: %s\n", TZ_CLIENT_FULL_PATH_DEV_NAME);
        goto out;
    }

    ret = ioctl(fd, 1, cmd);
    if (ret)
    {
        APP_ERR("os_ioctl error with code %d!\n", ret);
        close(fd);
        goto out;
    }

    close(fd);
    status = true;

out:
    if (err_no)
        *err_no = ret;

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

    if (errno == ENOMEM)
    {
        /* try increasing soft limit.
         * this will only work if this process is privileged
         * or if the soft limit is less than the hard limit
         */
        struct rlimit memlock_limit;

        if (getrlimit(RLIMIT_MEMLOCK, &memlock_limit))
        {
            APP_ERR("getrlimit");
            return -1;
        }
        memlock_limit.rlim_cur += len;
        if (setrlimit(RLIMIT_MEMLOCK, &memlock_limit))
        {
            if (!(memlock_limit.rlim_cur > memlock_limit.rlim_max && errno == EINVAL))
            {
                APP_ERR("setrlimit");
            }
            return -2;
        }

        /* successfully increased limit. try locking again. */
        if (mlock(ptr, len))
        {
            APP_ERR("mlock");
            return -3;
        }

        app_touch_mem(ptr, len);
        return 0;
    }

    APP_ERR("mlock");
    return -4;
}

int app_unlock_mem(const void *addr, size_t len)
{
    return munlock(addr, len);
}

vaddr_t app_mm_alloc(uint32_t size)
{
    void *result = NULL;

    result = malloc(size);

    bzero(result, size);
    return (vaddr_t)result;
}

void app_mm_free(vaddr_t vaddr)
{
    free((void *)vaddr);
}

static vaddr_t app_mm_alloc_aligned(uint32_t size, uint32_t alignment, struct mem_alignment_info *info)
{
    void *ptr = NULL;

    if (!info)
        return INVALID_VADDR;

    if (posix_memalign(&ptr, alignment, size))
    {
        APP_ERR("req aligned memory error!\n");
        return INVALID_VADDR;
    }

    bzero(ptr, size);

    info->_buf_start_vaddr = (vaddr_t)ptr;
    info->_buf_size = size;
    return (vaddr_t)ptr;
}

static void app_mm_free_aligned(vaddr_t vaddr, struct mem_alignment_info *info)
{
    (void)info;
    free((void *)vaddr);
}

vaddr_t app_mm_alloc_and_lock_4Kpages_for_sdom(uint32_t num_pages, struct mem_alignment_info *info)
{
    int ret = 0;
    vaddr_t vaddr = INVALID_VADDR;
    size_t len = 0;

    if (!num_pages || !info)
        return INVALID_VADDR;

    len = num_pages * PAGE_SIZE_4K;
    vaddr = app_mm_alloc_aligned(len, PAGE_SIZE_4K, info);
    app_mem_force_commit_in_pagetable((void *)vaddr, len);
    mfence();

    ret = app_lock_mem((void *)vaddr, len);
    if (ret)
    {
        APP_ERR("Allocate memory pages for S domain error: Failed to lock pages (vaddr:0x%lX, len:0x%lX)! ret:%d\n", vaddr, len, ret);
        app_mm_free_aligned(vaddr, info);
        return INVALID_VADDR;
    }

    mfence();

    // On success
    return vaddr;
}

void app_mm_free_and_unlock_4Kpages_for_sdom(struct mem_alignment_info *info)
{
    int ret = 0;

    if (!info)
        return;

    ret = app_unlock_mem((void *)info->_buf_start_vaddr, info->_buf_size);
    if (ret)
    {
        APP_WARN("Free memory pages for S domain error: Failed to unlock pages (vaddr:0x%lX, len:0x%lX)! ret:%d\n", info->_buf_start_vaddr, info->_buf_size, ret);
    }

    app_mm_free_aligned(info->_buf_start_vaddr, info);
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
    pid_t pid = 0;

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

    pid = fork();
    if (pid == -1)
    {
        // Fork failed
        APP_ERR("fork");
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
        APP_ERR("execv");
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