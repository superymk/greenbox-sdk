// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* Defines App - SecAgent Interfaces */

#ifndef APP_OS_INTERFACE_H
#define APP_OS_INTERFACE_H

#include <app_types.h>

/********* CPU Affinity of red application loader *********/
#define SP_LOADER_CPU_AFFINITY_CORE_1		(1)
#define SP_LOADER_CPU_AFFINITY_CORE_2		(2)
#define SP_LOADER_CPU_AFFINITY_CORE_3		(3)


#ifndef __ASSEMBLY__
struct tz_cmd_packet;
union sha_digest;

struct mem_alignment_info
{
    vaddr_t _buf_start_vaddr;
    size_t _buf_size;
};

extern uint64_t rdtscll(void);

extern void os_atomic_increase(uint32_t *var);
extern void os_atomic_decrease(uint32_t *var);

/// @brief Callback handler required in SecProcess loader to handle its exceptions
extern void sp_loader_on_exception(void);

extern void sp_loader_os_init();
extern void sp_loader_os_fini();

extern void sp_loader_install_exception_handler(void);

extern void msleep(uint32_t ms);
extern void press_continue(const char* prompt);

/// @brief Wait for users signal the loader to exit.
/// @param  
extern void app_wait_for_exit(void);

extern bool app_set_cpu_affinity(uint32_t cpu);
extern bool thread_set_high_priority(void);

/// @brief Un-schedule all processes from the CPU core <cpu> 
/// @param cpu 
/// @return 
extern bool kick_all_other_processes_from_core(uint32_t cpu);

extern int app_lock_mem(void* ptr, size_t len);
extern int app_unlock_mem(const void* addr, size_t len);
extern bool os_ioctl(struct tz_cmd_packet* cmd, int* err_no);
extern app_pid_t app_get_current_procid(void);

/// @brief Return the number of processors
/// @param  
/// @return 
uint32_t app_cpus_get_procs_num(void);

extern vaddr_t app_mm_alloc(uint32_t size);
extern void app_mm_free(vaddr_t vaddr);
extern vaddr_t app_mm_alloc_and_lock_4Kpages_for_sdom(uint32_t num_pages, struct mem_alignment_info* info);
extern void app_mm_free_and_unlock_4Kpages_for_sdom(struct mem_alignment_info* info);
extern void app_mm_memcpy(void* dst, const void* src, size_t size);
extern void* app_mm_memset(void* m, int c, size_t n);



static inline u16 app_pci_to_bdf(int bus, int dev, int fn) {
    return (bus<<8) | (dev<<3) | fn;
}

extern bool file_read(const char* filename, void* buf, uint32_t offset, size_t size);
extern bool file_get_size(const char* filename, size_t* out_sz);

/// @brief Execute a program as a child process and wait the process to exit
/// @param ignore_child_status <true> if the caller wants to ignore the child process's return status. 
/// @param exe_filepath 
/// @param exe_file_hash [Optional] If not NULL, then the function only execute the program if its hash matches <exe_file_hash>.
/// @param exe_name 
/// @param first_arg The argument must not be NULL. 
/// @param  
/// @return Return 0 if the child process exits with the code 0. 
///         Return the child process's exit codes on its errors
///         Return -1 otherwise.
extern int os_execute_program_wait_exit(bool ignore_child_status, const char* exe_filepath, const union sha_digest* exe_file_hash, const char* exe_name, const char *first_arg, ...);

/// @brief Execute a program as a child process but does not wait the process to exit.
/// @param ignore_child_status 
/// @param hide_window
/// @param exe_filepath 
/// @param exe_file_hash 
/// @param exe_name 
/// @param first_arg 
/// @param  
/// @return Return the process handle on success or (ulong_t)-1 on error.
extern ulong_t os_execute_program_nowait(
    bool ignore_child_status,
    bool hide_window,
    const char *exe_filepath,
    const union sha_digest *exe_file_hash,
    const char *exe_name,
    const char *first_arg,
    ...);

/// @brief Given the process‐handle returned from <os_execute_program_nowait>, terminate the process and closes the handle.
/// @param process_handle 
/// @return 0 on success, 1 on failure.
extern int os_program_kill(ulong_t process_handle);

/// @brief Create a lock on the file system 
/// @param filepath 
/// @param out_fd 
/// @return 
extern int os_file_lock_create(const char* filepath, ulong_t* out_fd);

/// @brief Release a lock on the file system.
/// @param fd 
/// @return 
extern int os_file_lock_release(ulong_t fd);


extern int fs_create_directories(const char *filepath);
extern FILE* fopen_normalized_path(const char *filepath, const char* mode);
extern int fs_read_file(const char *filepath, uint8_t create_if_not_exist, uint8_t **data, size_t *len);
extern int fs_write_file(const char *filepath, const uint8_t *data, size_t len);
extern int fs_delete_file(const char *filepath);
extern int fs_file_get_size(const char* filepath, size_t* out_sz);
extern int fs_chmod_file_0664(const char *filepath);




/********* OS Sound Functions *********/

/// @brief Mute the untrusted OS. In case of nested calls to this function, only the first call is effective.
/// @param owner_lock A pointer that could identify a caller; e.g., a pointer of a local variable or a function pointer. 
/// Calling to this function may be nested; i.e., another OS mute is requested before unmuting the OS. Only the first 
/// caller can actually mute/unmute the OS.
/// @param out_already_muted
/// @return 
extern int os_sound_mute(const void* owner_lock, bool* out_already_muted);

/// @brief Unmute the untrusted OS. 
/// @param owner_lock A pointer that could identify a caller; e.g., a pointer of a local variable or a function pointer. 
/// Calling to this function may be nested; i.e., another OS mute is requested before unmuting the OS. Only the first 
/// caller can actually mute/unmute the OS.
/// @param already_muted
/// @return 
extern int os_sound_unmute(const void* owner_lock, bool already_muted);

#endif // __ASSEMBLY__
#endif // APP_OS_INTERFACE_H