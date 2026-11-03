// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef PAL_GLIBC_SYSCALL_H
#define PAL_GLIBC_SYSCALL_H

#ifndef __ASSEMBLY__
#include <vk_syscalls.h>

// Return the current SecProcess
extern SP_UTASK* current_sp;


extern ulong_t sp_sysenter(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3);
extern void sos_sys_sp_exit(SAP_STATUS exit_status);

/// @brief Return the information of the target SecProcess.
/// @param target_sp_id ID of the target SecProcess.
/// @param out_result The vaddr of the output result.
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
///         Return EINTERNAL on internal errors.
extern VKSTATUS sos_sys_sp_get_info(spid_t target_sp_id, SOS_SP_GET_INFO_RESP* out_result);
extern spid_t sos_sys_sp_query_id(enum sp_type sp_type, const char* sp_name, bool is_trusted);

/********* Syscalls - uTPM *********/
/// @brief Invoke SecBase to set the uTPM's secret and re-initialize uTPM's sealed storage functions based on the new
/// secret. Only a subset of trusted SecProcesses can issue this system call.
/// @param secret Caller SecProcess's memory buffer of the new secret.
/// @return Return SUCCESS on success.
///         Return ENOMEM on memory allocation failure.
/// 		Return EPERM if SecProcess is not allowed to invoke this system call.
///         Return EINTERNAL on uTPM set secret errors. 
extern VKSTATUS sos_sys_utpm_set_secret_and_re_init(uint8_t* secret);

/// @brief Return the information of the uTPM.
/// @param out_info Pointer to the result buffer allocated by the caller.
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to access the output memory.
///         Return EINTERNAL on internal errors.
extern VKSTATUS sos_sys_utpm_get_info(SOS_uTPM_INFO* out_info);

/// @brief Set measurements of the given SecApp (i.e., image and secure parameters) into the given Platform 
/// Configuration Registers (PCRs) of the uTPM. Only trusted SecProcesses can issue this system call.
/// [NOTE] Setting measurements to a given value is equivilant to resetting and extending a PCR in security, though 
/// the result value is different.
/// @param target_sp_id ID of the target SecProcess.
/// @param pcr_image uTPM PCR for the SecApp's image. The value must be within [28, 32).
/// @param pcr_sparams uTPM PCR for the SecApp's secure parameters. The value must be within [28, 32).
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to invoke this system call.
///         Return EINTERNAL on uTPM extend errors.
extern VKSTATUS sos_sys_utpm_set_sp_measurement(spid_t target_sp_id, uint32_t pcr_image, uint32_t pcr_sparams);

/// @brief Reset the measurements in the given Platform Configuration Registers (PCRs) of the uTPM. Only trusted 
/// SecProcesses can issue this system call.
/// @param pcr_image uTPM PCR 1 to reset. The value must be within [28, 32).
/// @param pcr_sparams uTPM PCR 2 to reset. The value must be within [28, 32).
/// @return Return SUCCESS on success.
///         Return EINVAL on invalid arguments.
/// 		Return EPERM if SecProcess is not allowed to invoke this system call.
///         Return EINTERNAL on uTPM extend errors.
extern VKSTATUS sos_sys_utpm_reset_sp_measurement(uint32_t pcr_image, uint32_t pcr_sparams);

/// @brief Get a quote of all PCRs in the uTPM.
/// @param nonce_str Nonce string.
/// @param nonce_str_len Length of the nonce string.
/// @param nonce_timestamp Additional timestamp for the nonce.
/// @param out_result Caller SecProcess's memory buffer to store the quote report.
/// @return Return SUCCESS on success.
///         Return ENOMEM on memory allocation failure.
/// 		Return EPERM if SecProcess is not allowed to access the destination memory buffer.
///         Return EINTERNAL on uTPM quote errors.
extern VKSTATUS sos_sys_utpm_get_quote(char* nonce_str, size_t nonce_str_len, uint64_t nonce_timestamp, SOS_uTPM_GET_QUOTE_RESP* out_result);

extern void sos_sys_sp_yield(spid_t switch_to);

/// @brief Enable SecProcess scheduling on the current CPU
/// [NOTE] All SecProcesses are allowed to invoke this system call, because even SecApps (not just SecServices) could have
/// critical sections. For example, when SecApp A calls SecApp B, B needs to disable scheduling first, or A may be 
/// scheduled before B finishes its service and causes B or A crashes.
///
/// @return Return SUCCESS on success. 
///         Return EINVAL if the current CPU structure cannot be found
// extern VKSTATUS sos_sys_sp_scheduling_enable(void);

/// @brief Disable SecProcess scheduling on the current CPU
/// [NOTE] All SecProcesses are allowed to invoke this system call, because even SecApps (not just SecServices) could have
/// critical sections. For example, when SecApp A calls SecApp B, B needs to disable scheduling first, or A may be 
/// scheduled before B finishes its service and causes B or A crashes.
///
/// @return Return SUCCESS on success. 
///         Return EINVAL if the current CPU structure cannot be found
// extern VKSTATUS sos_sys_sp_scheduling_disable(void);




/********* Syscalls - Time *********/
/// SecOS defines multiple terms for different timers:
/// Global time: The time elapsed after SecOS boots. It includes all the time spent on both the S domain and the NS 
/// domain after SecOS boots. The unit is second (e.g., s/ms/us/ns).
/// Wall clock time: Unix (wall clock) time. It is the seconds that have elapsed since 1970/01/01.
/// NS domain's time: The time spent in the untrusted OS/Apps after SecOS boots.

extern VKSTATUS sos_sys_get_current_cpu_boot_freq(uint64_t* out_freq);

extern VKSTATUS sos_sys_time_init_wall_clock(TIMEVAL* tv, TIMEZONE* tz);

/// @brief Get the wall clock timer in SecOS.
/// [NOTE] The returned value may be inaccurate and untrustworthy, because the untrusted OS tells SecOS about the initial
/// wall clock value.
/// @param tv 
/// @param tz 
/// @return 
extern VKSTATUS sos_sys_time_get_wall_clock(TIMEVAL* tv, TIMEZONE* tz);

/// @brief Get the current SecOS kernel time.
/// [NOTE] The returned value may be inaccurate and untrustworthy. [TODO][Issue 170].
/// @param result 
/// @return 
extern VKSTATUS sos_sys_time_get_global(sos_timespec_t* result);




/********* Syscalls - Pseudo-Random Number Generator *********/
/// @brief Return a pseudo-random number seeded by HW.
/// @param len length of the random number. The number must <= 1024.
/// @param buf vaddr of the result buffer.
/// @return Return SUCCESS on success.
///         Return other values on errors.
extern VKSTATUS sos_sys_hw_prng_get_random_number(uint32_t len, uint8_t* buf);




/********* Syscalls - Memory Translation *********/
extern paddr_t sos_sys_mm_vaddr_to_paddr(vaddr_t vaddr);
extern void sos_sys_print_str(const char* str, size_t len);

/// @brief Flush a range of current SecProcess's memory, or flush all memory if it fails.
///
/// @param arg1 virtual address of the current SecProcess's memory.
/// @param arg2 memory size.
/// 
/// @return Return SUCCESS on success.
///			Return EINVAL if the current SecProcess is invalid.
/// 		Return EPERM if the current SecProcess is not allowed to access the memory.
extern VKSTATUS sos_sys_mm_flush_cache_range(vaddr_t vaddr, size_t mem_sz);

/// @brief Return true if the current SecProcess can read and write the target memory [vaddr, vaddr + mem_sz)
/// @param vaddr 
/// @param mem_sz 
/// @return 
extern bool sos_sys_mm_check_allow_rw(vaddr_t vaddr, size_t mem_sz);




/********* Syscalls - Object Management *********/
/// @brief Create a root object; i.e., object without a parent object.
///
/// @param addr Address of physical memory of the S domain, or address of system physical memory (spaddr_t) of the 
///         machine, or address of IRQ objects. Ignored if creating other types of objects.
/// @param size Size of memory and IRQ objects. Ignored if creating other types of objects.
/// @param flags Creation flags. Note: <ns_access_type> is valid only for OBJ_MEM_PLATFORM root object.
///         Creation of any child objects works as <ns_access_type> is <OBJECT_ACCESS_NS_ACCESS_MEDIATED>.
///
/// @return Return the handle of created object.
///			Return INVALID_HANDLE on any errors.
extern shandle_t sos_sys_obj_create(addr_t addr, size_t size, SYS_PARAMS_OBJECT_CREATE_ROOT_OBJECT_FLAGS flags);

/// @brief Destroy an object.
///
/// @param arg1 Object handle.
///
/// @return Returns SUCCESS if succeed.
///  		Returns E_OBJ_NOTOWNED if <owner_pool> does not own the object <h_obj>.
/// 		Returns EINVAL if any parameters are empty or on any other internal errors.
extern VKSTATUS sos_sys_obj_destroy(shandle_t h_object);

/// @brief Get the information of the given object. Note that the caller SecProcess must have registered the object, or
/// the system call returns errors.
///
/// @param arg1 Object handle.
/// @param arg2 Buffer to hold the result info.
///
/// @return Return SUCCESS on success.
///			Return E_OBJ_NOTEXIST if the SecProcess has not registered the given object.
///			Return EINTERNAL on other errors.
extern VKSTATUS sos_sys_obj_get_info(shandle_t h_object, SYS_OBJ_INFO* out_info);

/// @brief Take a non-shared object and create a child object from it. All resources not given to the child still
/// belong to the parent.
/// [NOTE] Only the object owner can call this function.
/// [NOTE] The parent object and its ancestors must not be shared.
/// [NOTE] If the parent object is an OBJ_MEM_PLATFORM object with <ns_access_type> flag set to BLOCKED/PASSTHRU on creation, the
/// flag is NOT inherited to the child objects. Caller SecProcesses must make sure the creation of child objects does not
/// break the NS domain's accesses to the objects. Otherwise, the caller must not create child objects and build
/// their own OBJ_MEM_PLATFORM memory/object management for the parent object.
///
/// @param h_object Object handle.
/// @param new_addr_start Start address of the child object's resource.
/// @param new_size Size of the child object's resource.
///
/// @return Return the handle of the child object on success. 
///			Return INVALID_HANDLE on errors.
extern shandle_t sos_sys_obj_split_object(shandle_t h_object, addr_t new_addr_start, size_t new_size);

/// @brief Grant access permission of an owned object to a user.
/// [NOTE] Only the object owner can grant access permission to a user. 
/// [NOTE] A SecProcess cannot share an object with a same SecProcess twice without removing the share first.
/// [NOTE] The object must not have any child objects.
///
/// @param h_object Object handle.
/// @param target_sp_id Target SecProcess.
/// @param max_access_perm Desired access permission.
///
/// @return Return SUCCESS on success.
///			Return EINVAL if the target SecProcess does not have an object pool. In other words, the target SecProcess is malformed.
///			Return EINTERNAL on other errors.
extern VKSTATUS sos_sys_obj_share_sp(shandle_t h_object, spid_t target_sp_id, OBJ_ACCESS_MODE max_access_perm);

/// @brief Remove access permission of an owned object from a user.
/// [NOTE] Only the object owner can grant access permission to a user.
/// [NOTE] A SecProcess cannot share an object with a same SecProcess twice without removing the share first.
/// [NOTE] The object must not have any child objects.
///
/// @param h_object Object handle.
/// @param target_sp_id Target SecProcess.
///
/// @return Return SUCCESS on success.
///			Return EINVAL if the target SecProcess does not have an object pool. In other words, the target SecProcess is malformed.
///			Return EINTERNAL on other errors.
extern VKSTATUS sos_sys_obj_unshare_sp(shandle_t h_object, spid_t target_sp_id);

/// @brief Reference a resource object in the SecProcess, so it can be directly addressed later.
///
/// @param h_object Object handle.
/// @param map_vaddr Virtual address of the SecProcess to be mapped.
/// @param desired_perm Desired access permission.
///
/// @return Return SUCCESS on success.
///			Return EPERM if the desired permission is not allowed.
///			Return EINTERNAL on other errors.
extern VKSTATUS sos_sys_obj_reference_object(shandle_t h_object, vaddr_t map_vaddr, uint32_t desired_perm);

/// @brief Unreference a resource object in the SecProcess.
///
/// @param arg1 Object handle.
///
/// @return Return SUCCESS.
extern VKSTATUS sos_sys_obj_unreference_object(shandle_t h_object);

/// @brief Re-reference the given resource object in the SecProcess.
/// @param h_object 
/// @param map_vaddr 
/// @param desired_perm 
/// @return 
extern VKSTATUS sos_sys_obj_re_reference_object(shandle_t h_object, vaddr_t map_vaddr, uint32_t desired_perm);




/********* Syscalls - SecProcess Communication *********/
extern void sos_sys_sp_register_sig_handler(vaddr_t handler);
extern uint32_t sos_sys_sp_signal_ret(vaddr_t current_stack_addr);
extern void sos_sys_switch_to_secprocess_event_handling(spid_t target_sp_id);

/// @brief Initialize the communication between two SecProcesses
///
/// @param dest_spid ID of the other communicating SecProcess.
/// @param h_parent_mem Handle of the parent object of the memory buffer.
/// @param comm_buf Memory for communications. It must be PAGE_SIZE_4K aligned.
/// @param num_pages Number of 4K pages.
///
/// @return Return the actual handle on success.
///			Return INVALID_HANDLE on any errors.
extern shandle_t sos_sys_comm_isp_create(spid_t dest_spid, shandle_t h_parent_mem, vaddr_t comm_buf, uint32_t num_pages);

/// @brief Destroy the communication channel. The caller SecProcess must be 
/// the owner of the communication channel (the one that allocates the
/// communication buffer)
///
/// @return Return SUCCESS on success.
///			Return EINTERNAL otherwise.
extern uint32_t sos_sys_comm_isp_destroy(shandle_t h_comm_sp);

/// @brief Send the SecProcess with an event. This function does not wait for the response.
/// [NOTE] This system call returns nothing, SecProcesses should use their own Inter-SecProcess protocols to check
/// communication status.
///
/// @return Nothing.
extern void sos_sys_comm_isp_send(COMM_ISP_SEND_PARAMS* params);

/// @brief Call and switch to the target SecProcess.
/// [NOTE] This system call returns nothing, SecProcesses should use their own Inter-SecProcess protocols to check
/// communication status.
///
/// @return Nothing.
extern void sos_sys_comm_isp_call(COMM_ISP_SEND_PARAMS* params);

extern void sos_sys_notify_apploader_sp_running(ulong_t info);




/********* Syscalls - I/O Accesees *********/
extern void sos_sys_enable_intr(void);
extern void sos_sys_disable_intr(void);
extern VKSTATUS sos_sys_portio_read(uint16_t port, uint8_t read_size, uint32_t* buf);
extern VKSTATUS sos_sys_portio_write(uint16_t port, uint32_t value, uint8_t write_size);




/********* Syscalls - Hypercalls for I/O devices *********/
extern VKSTATUS sos_sys_platform_get_nonsensitive_info(SECBASE_PLATFORM_NONSENSITIVE_INFO* out_info);




/********* Syscalls - System quirks *********/

#ifdef GREENBOX_USE_UXMHF64_V1
//// [TODO] These quirks should be fixed in the future
extern VKSTATUS sos_sys_quirks_get_info(SECOS_QUIRKS_INFO* out_info);
#endif // GREENBOX_USE_UXMHF64_V1

#endif // __ASSEMBLY__
#endif // PAL_GLIBC_SYSCALL_H