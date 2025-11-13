// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers_libc.h>

void sos_sys_sp_exit(SAP_STATUS exit_status)
{
    (void)sp_sysenter(SYS_SP_SELF_EXIT, exit_status, 0, 0);
    asm volatile("ud2");
}

VKSTATUS sos_sys_sp_get_info(spid_t target_sp_id, SOS_SP_GET_INFO_RESP *out_result)
{
    return sp_sysenter(SYS_SP_GET_INFO, (ulong_t)target_sp_id, (ulong_t)out_result, 0);
}

spid_t sos_sys_sp_query_id(enum sp_type sp_type, const char *sp_name, bool is_trusted)
{
    return sp_sysenter(SYS_SP_QUERY_ID, (ulong_t)sp_type, (ulong_t)sp_name, (ulong_t)is_trusted);
}


/********* Syscalls - uTPM *********/
VKSTATUS sos_sys_utpm_set_secret_and_re_init(uint8_t* secret)
{
    return (VKSTATUS)sp_sysenter(SYS_uTPM_SET_SECRET_AND_REINIT_STORAGE_OPS, (ulong_t)secret, 0, 0);
}

VKSTATUS sos_sys_utpm_get_info(SOS_uTPM_INFO* out_info)
{
    // Check: params must be valid
    if (!out_info)
        return EINVAL;

    return (VKSTATUS)sp_sysenter(SYS_uTPM_GET_INFO, (ulong_t)out_info, 0, 0);
}

VKSTATUS sos_sys_utpm_set_sp_measurement(spid_t target_sp_id, uint32_t pcr_image, uint32_t pcr_sparams)
{
    return (VKSTATUS)sp_sysenter(SYS_uTPM_RESET_AND_EXTEND_SP_HASH, (ulong_t)target_sp_id, (ulong_t)pcr_image, (ulong_t)pcr_sparams);
}

VKSTATUS sos_sys_utpm_reset_sp_measurement(uint32_t pcr_image, uint32_t pcr_sparams)
{
    return (VKSTATUS)sp_sysenter(SYS_uTPM_RESET_SP_HASH, (ulong_t)pcr_image, (ulong_t)pcr_sparams, 0);
}

VKSTATUS sos_sys_utpm_get_quote(char *nonce_str, size_t nonce_str_len, uint64_t nonce_timestamp, SOS_uTPM_GET_QUOTE_RESP *out_result)
{
    SOS_uTPM_GET_QUOTE_REQ req = {0};
    size_t nonce_str_mem_len = nonce_str_len + 1;

    // Check: params must be valid
    if (!nonce_str || !out_result)
        return EINVAL;

    if (nonce_str_mem_len >= UTPM_QUOTE_NONCE_STRING_MAX_LENGTH)
        return EINVAL;

    memcpy(req.nonce_str, nonce_str, nonce_str_mem_len);
    *(uint64_t *)req.nonce_freshness = nonce_timestamp;

    return (VKSTATUS)sp_sysenter(SYS_uTPM_GET_QUOTE, (ulong_t)&req, (ulong_t)out_result, 0);
}

void sos_sys_sp_yield(spid_t switch_to)
{
    sp_sysenter(SYS_SP_YIELD_CPU, (ulong_t)switch_to, 0, 0);
}

/********* Syscalls - Time *********/
VKSTATUS sos_sys_get_current_cpu_boot_freq(uint64_t *out_freq)
{
    return (VKSTATUS)sp_sysenter(SYS_GET_CURRENT_CPU_BOOT_FREQ, (ulong_t)out_freq, 0, 0);
}

VKSTATUS sos_sys_time_init_wall_clock(TIMEVAL *tv, TIMEZONE *tz)
{
    return (VKSTATUS)sp_sysenter(SYS_TIME_INIT_WALL_CLOCK, (ulong_t)tv, (ulong_t)tz, 0);
}

VKSTATUS sos_sys_time_get_wall_clock(TIMEVAL *tv, TIMEZONE *tz)
{
    return (VKSTATUS)sp_sysenter(SYS_TIME_GET_WALL_CLOCK, (ulong_t)tv, (ulong_t)tz, 0);
}

VKSTATUS sos_sys_time_get_global(sos_timespec_t *result)
{
    return (VKSTATUS)sp_sysenter(SYS_TIME_GET_GLOBAL, (ulong_t)result, 0, 0);
}

/********* Syscalls - Pseudo-Random Number Generator *********/
VKSTATUS sos_sys_hw_prng_get_random_number(uint32_t len, uint8_t *buf)
{
    return (VKSTATUS)sp_sysenter(SYS_HW_PRNG_GET_RANDOM_NUMBER, (ulong_t)len, (ulong_t)buf, 0);
}

/********* Syscalls - Memory Translation/Access *********/
paddr_t sos_sys_mm_vaddr_to_paddr(vaddr_t vaddr)
{
    return sp_sysenter(SYS_MM_VADDR_TO_PADDR, (ulong_t)vaddr, 0, 0);
}

void sos_sys_print_str(const char *str, size_t len)
{
    sp_sysenter(SYS_SP_PRINT, (ulong_t)str, (ulong_t)len, 0);
}

VKSTATUS sos_sys_mm_flush_cache_range(vaddr_t vaddr, size_t mem_sz)
{
    return (VKSTATUS)sp_sysenter(SYS_MM_FLUSH_CACHE_RANGE, (ulong_t)vaddr, (ulong_t)mem_sz, 0);
}

bool sos_sys_mm_check_allow_rw(vaddr_t vaddr, size_t mem_sz)
{
    VKSTATUS status = SUCCESS;
    
    status = (VKSTATUS)sp_sysenter(SYS_MM_CHECK_ALLOW_RW, (ulong_t)vaddr, (ulong_t)mem_sz, 0);
    if(FAIL(status))
    {
        // If SecOS returns any errors, the SecProcess may not read or write the target memory.
        return false;
    }

    // SecProcess has the read and write permissions.
    return true; 
}

/********* Syscalls - Object Management *********/
shandle_t sos_sys_obj_create(addr_t addr, size_t size, SYS_PARAMS_OBJECT_CREATE_ROOT_OBJECT_FLAGS flags)
{
    return (shandle_t)sp_sysenter(SYS_OBJ_CREATE_ROOT_OBJECT, (ulong_t)addr, (ulong_t)size, (ulong_t)flags.raw);
}

VKSTATUS sos_sys_obj_destroy(shandle_t h_object)
{
    return (VKSTATUS)sp_sysenter(SYS_OBJ_DESTROY_OBJECT, (ulong_t)h_object, 0, 0);
}

VKSTATUS sos_sys_obj_get_info(shandle_t h_object, SYS_OBJ_INFO *out_info)
{
    return (VKSTATUS)sp_sysenter(SYS_OBJ_GET_INFO, (ulong_t)h_object, (ulong_t)out_info, 0);
}

shandle_t sos_sys_obj_split_object(shandle_t h_object, addr_t new_addr_start, size_t new_size)
{
    return (shandle_t)sp_sysenter(SYS_OBJ_SPLIT_OBJECT, (ulong_t)h_object, (ulong_t)new_addr_start, (ulong_t)new_size);
}

VKSTATUS sos_sys_obj_share_sp(shandle_t h_object, spid_t target_sp_id, OBJ_ACCESS_MODE max_access_perm)
{
    return (VKSTATUS)sp_sysenter(SYS_OBJ_SHARE_OBJECT_SP, (ulong_t)h_object, (ulong_t)target_sp_id, (ulong_t)max_access_perm);
}

VKSTATUS sos_sys_obj_unshare_sp(shandle_t h_object, spid_t target_sp_id)
{
    return (VKSTATUS)sp_sysenter(SYS_OBJ_UNSHARE_OBJECT_SP, (ulong_t)h_object, (ulong_t)target_sp_id, 0);
}

VKSTATUS sos_sys_obj_reference_object(shandle_t h_object, vaddr_t map_vaddr, uint32_t desired_perm)
{
    return (VKSTATUS)sp_sysenter(SYS_OBJ_REFERENCE_OBJECT, (ulong_t)h_object, (ulong_t)map_vaddr, (ulong_t)desired_perm);
}

VKSTATUS sos_sys_obj_unreference_object(shandle_t h_object)
{
    return (VKSTATUS)sp_sysenter(SYS_OBJ_UNREFERENCE_OBJECT, (ulong_t)h_object, 0, 0);
}

VKSTATUS sos_sys_obj_re_reference_object(shandle_t h_object, vaddr_t map_vaddr, uint32_t desired_perm)
{
    VKSTATUS status = SUCCESS;

    status = sos_sys_obj_unreference_object(h_object);
    if (FAIL(status))
    {
        printf("Re-reference the object (%u) error: Failed to unreference the object!\n", h_object);
        return status;
    }

    return sos_sys_obj_reference_object(h_object, map_vaddr, desired_perm);
}

/********* Syscalls - SecProcess Communication *********/
void sos_sys_sp_register_sig_handler(vaddr_t handler)
{
    sp_sysenter(SYS_SIGNAL_INSTALL_HANDLER, (ulong_t)handler, 0, 0);
}

uint32_t sos_sys_sp_signal_ret(vaddr_t current_stack_addr)
{
    return sp_sysenter(SYS_SIGNAL_RETURN, (ulong_t)current_stack_addr, 0, 0);
}

/// @brief Switch to the target SecProcess's event handling.
/// [NOTE] This system call returns nothing, because SecProcesses should use their own Inter-SecProcess protocols to check
/// communication status.
///
/// @param arg1 is the ID of the target SecProcess
///
/// @return Nothing
void sos_sys_switch_to_secprocess_event_handling(spid_t target_sp_id)
{
    sp_sysenter(SYS_SWITCH_TO_SP_EVENT_HANDLING, target_sp_id, 0, 0);
}

/// @brief Initialize the communication between two SecProcesses
///
/// @param dest_spid is the spid of the other communicating SecProcess.
/// @param h_parent_mem is the parent object of the memory buffer.
/// @param comm_buf must be PAGE_SIZE_4K aligned.
/// @param num_pages represent the number of 4K pages.
///
/// @return Return the actual handle on success.
///			Return INVALID_HANDLE on any errors.
shandle_t sos_sys_comm_isp_create(spid_t dest_spid, shandle_t h_parent_mem, vaddr_t comm_buf, uint32_t num_pages)
{
    COMM_ISP_CONF comm_isp_conf;

    comm_isp_conf.target_spid = dest_spid;
    comm_isp_conf.h_mem = h_parent_mem;
    comm_isp_conf.comm_buf = comm_buf;
    comm_isp_conf.num_pages = num_pages;

    mfence();
    return sp_sysenter(SYS_COMM_ISP_CREATE, (ulong_t)&comm_isp_conf, 0, 0);
}

/// @brief Destroy the communication channel. The caller SecProcess must be
/// the owner of the communication channel (the one that allocates the
/// communication buffer)
///
/// @return Return SUCCESS on success.
///			Return EINTERNAL otherwise.
uint32_t sos_sys_comm_isp_destroy(shandle_t h_comm_sp)
{
    return sp_sysenter(SYS_COMM_ISP_DESTROY, (ulong_t)h_comm_sp, 0, 0);
}

void sos_sys_comm_isp_send(COMM_ISP_SEND_PARAMS *params)
{
    sp_sysenter(SYS_COMM_ISP_SEND, (ulong_t)params, 0, 0);
}

void sos_sys_comm_isp_call(COMM_ISP_SEND_PARAMS *params)
{
    sp_sysenter(SYS_COMM_ISP_CALL, (ulong_t)params, 0, 0);
}

void sos_sys_notify_apploader_sp_running(ulong_t info)
{
	sp_sysenter(SYS_NOTIFY_APPLOADER_SP_RUNNING, info, 0, 0);
}

/********* Syscalls - I/O Accesees *********/
void sos_sys_enable_intr(void)
{
    sp_sysenter(SYS_IO_ENABLE_INTR, 0, 0, 0);
}

void sos_sys_disable_intr(void)
{
    sp_sysenter(SYS_IO_DISABLE_INTR, 0, 0, 0);
}

VKSTATUS sos_sys_portio_read(uint16_t port, uint8_t read_size, uint32_t *buf)
{
    return (VKSTATUS)sp_sysenter(SYS_PORTIO_READ, (ulong_t)port, (ulong_t)read_size, (ulong_t)buf);
}

VKSTATUS sos_sys_portio_write(uint16_t port, uint32_t value, uint8_t write_size)
{
    return (VKSTATUS)sp_sysenter(SYS_PORTIO_WRITE, (ulong_t)port, (ulong_t)value, (ulong_t)write_size);
}

/********* Syscalls - Hypercalls for I/O devices *********/
VKSTATUS sos_sys_platform_get_nonsensitive_info(SECBASE_PLATFORM_NONSENSITIVE_INFO *out_info)
{
    return (VKSTATUS)sp_sysenter(SYS_PLATFORM_GET_NONSENSITIVE_INFO, (ulong_t)out_info, 0, 0);
}

/********* Syscalls - System quirks *********/
#ifdef GREENBOX_USE_UXMHF64_V1
//// [TODO] These quirks should be fixed in the future
VKSTATUS sos_sys_quirks_get_info(SECOS_QUIRKS_INFO *out_info)
{
    return (VKSTATUS)sp_sysenter(SYS_QUIRKS_GET_INFO, (ulong_t)out_info, 0, 0);
}
#endif // GREENBOX_USE_UXMHF64_V1