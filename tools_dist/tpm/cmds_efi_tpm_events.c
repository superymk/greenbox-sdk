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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <log.h>

#include "cmds.h"
#include "app_os_if.h"
#include "utils_string.h"
#include "utils.h"

#include "./efi_event_log/tpm2_event_log.h"

#define EFI_EVENT_LOGS_FILENAME     "efi_event_logs.dat"

#define IS_VALID_PCR(pcr) (((uint32_t)(pcr)) < 24)

#ifdef __WINDOWS__

static int _efi_tpm2_evl_dump(void)
{
    int ret = 0;
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char efi_evl_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* exe_filename = "efi_tpm2_event_logs.exe";
    const char* efi_evl_filename = EFI_EVENT_LOGS_FILENAME;

    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);

    strncpy(efi_evl_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(efi_evl_filepath, efi_evl_filename);

    const union sha_digest _file_win_efi_tpm2_event_logs_exe_sha256 = {
        .sha2_256_digest = {
            GB_TPM_SUBTOOLS_EXECUTABLE_SHA256
        }
    };

    ret = os_execute_program_wait_exit(false, exe_filepath, &_file_win_efi_tpm2_event_logs_exe_sha256, exe_filename, efi_evl_filepath, NULL);
    if(ret)
    {
        printf("TPM Tool: Dump UEFI TPM2 event logs error! ret:%d\n", ret);
        return -1;
    }

    // On success
    return 0;
}

#elif defined (__LINUX__)

static int _efi_tpm2_evl_dump(void)
{
    int ret = 0;
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char efi_evl_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* exe_filename = "linux_efi_tpm2_evl_dump.sh";
    const char* efi_evl_filename = EFI_EVENT_LOGS_FILENAME;

    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);

    strncpy(efi_evl_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(efi_evl_filepath, efi_evl_filename);

    const union sha_digest _file_win_efi_tpm2_event_logs_exe_sha256 = {
        .sha2_256_digest = {
            0xe4, 0x94, 0x33, 0xfa, 0x00, 0x79, 0x6d, 0x14,
            0xe5, 0x2e, 0x23, 0x9d, 0x8e, 0x32, 0x89, 0xa5,
            0x06, 0x00, 0x5a, 0xf5, 0x93, 0x36, 0x24, 0x24,
            0x00, 0x5f, 0x47, 0x1a, 0x55, 0x17, 0x46, 0x51
        }
    };

    ret = os_execute_program_wait_exit(false, exe_filepath, &_file_win_efi_tpm2_event_logs_exe_sha256, exe_filename, efi_evl_filepath, NULL);
    if(ret)
    {
        printf("TPM Tool: Dump UEFI TPM2 event logs error! ret:%d\n", ret);
        return -1;
    }

    // On success
    return 0;
}

#endif // __WINDOWS__



static int _efi_tpm2_evl_parse(uint32_t pcr_idx, SECBASE_TPM_EVENT_LOG_TRACE* out_trace)
{
    int ret = 0;
    char efi_evl_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* efi_evl_filename = EFI_EVENT_LOGS_FILENAME;
    uint8_t *file_data = NULL;;
    size_t file_size = 0;

    strncpy(efi_evl_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(efi_evl_filepath, efi_evl_filename);

    ret = fs_read_file(efi_evl_filepath, false, &file_data, &file_size);
    if(ret)
    {
        printf("Read EFI event logs error! ret:%d \n", ret);
        return 1;
    }

    // Parse
    ret = efi_event_tpm2_get_pcr_extend_trace(file_data, file_size, pcr_idx);
    if(ret)
    {
        printf("Parse EFI event logs error! ret:%d \n", ret);
        return 1;
    }

    // Return the result
    memcpy(out_trace, tpm_event_log_get_trace(), sizeof(SECBASE_TPM_EVENT_LOG_TRACE));

    // On success
    return 0;
}

/************ Public Functions **********/
int cmd_efi_tpm2_event_logs_get_pcr_trace(uint32_t pcr_idx, SECBASE_TPM_EVENT_LOG_TRACE* out_trace)
{
    int ret = 0;

    // Check: params must be valid
    if(!IS_VALID_PCR(pcr_idx))
        return 1;
    if(!out_trace)
        return 1;

    ret = _efi_tpm2_evl_dump();
    if(ret)
    {
        printf("TPM Tool: Dump UEFI event logs error! ret:%d\n", ret);
        return ret;
    }

    ret = _efi_tpm2_evl_parse(pcr_idx, out_trace);
    if(ret)
    {
        printf("TPM Tool: Parse UEFI event logs error! ret:%d\n", ret);
        return ret;
    }

    // On success
    return 0;
}

uint32_t cmd_efi_tpm2_event_logs_get_pcr_trace_length(SECBASE_TPM_EVENT_LOG_TRACE* trace)
{
    // Check: params must be valid
    if(!trace)
        return 0;

    // Check: <trace->count> must be valid
    if(trace->count > SECBASE_TPM_EVENT_ENTRIES_MAX)
        return 0;

    return sizeof(trace->count) + trace->count * sizeof(SECBASE_TPM_EVENT_LOG_ENTRY);
}