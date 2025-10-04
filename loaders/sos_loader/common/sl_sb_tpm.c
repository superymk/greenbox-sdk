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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sl_headers.h>

#include <elf.h>
#include <secsrvs/sl_secsrvs.h>

#include <dev/usb.h>

#include "../hash/hash.h"
#include "sl_sb_tpm.h"
#include "base64.h"

// [TODO][Issue 159] File hash checks can be removed after the installer sets up file permissions correctly.

// SHA256 of tools/tpmtools/gb_tpm executable
const union sha_digest _file_gb_tpm_sha256 = {
    .sha2_256_digest = {
        GB_TPM_EXECUTABLE_SHA256
    }
};

__attribute__ ((unused)) static void replaceEqualsWithAt(char *str) 
{
    while (*str != '\0') {
        if (*str == '=') {
            *str = '@';
        }
        str++;
    }
}

/********* SecBase TPM Functions *********/
#define PARAM_PCR_LENGTH    (8)

SL_STATUS _do_sl_secbase_tpm_create_keys(void)
{
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char* param1 = "empty";

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
    {
        SL_ERR("<sl_secbase_tpm_seal> SecAgent is not loaded. Exit!\n");
        return SL_EINVAL;
    }

    // Invoke gb_tpm.exe to seal SecBase's secrets and save the output file alongside UXMHF64 files
    {

#if defined(__WINDOWS__)
        const char *exe_filename = "gb_secbase_tpm_create_keys.bat";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xbc, 0xde, 0x01, 0xf8, 0x68, 0x80, 0x86, 0xa5,
                0x07, 0x35, 0x6a, 0xd9, 0x30, 0x34, 0xeb, 0x5c,
                0x37, 0x05, 0xe7, 0xff, 0x51, 0x5a, 0x39, 0x3f,
                0xd6, 0x79, 0xa7, 0xfa, 0x34, 0x7a, 0x6f, 0x0b
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_create_keys.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0x5d, 0xcd, 0xb8, 0x96, 0xc4, 0xcd, 0x67, 0x82,
                0x86, 0xc4, 0x64, 0x47, 0x93, 0xb8, 0x12, 0xc9,
                0x66, 0x8e, 0xb8, 0xf8, 0xc9, 0x0f, 0x19, 0x47,
                0x29, 0xe1, 0x71, 0x69, 0xaa, 0xd0, 0x39, 0x78
            }
        };
#endif // __WINDOWS__
        int ret = 0;

        // Run the script to prepare GreenBox.
        strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(exe_filepath, exe_filename);
        ret = os_execute_program_wait_exit(true, exe_filepath, &_file_script_sha256, exe_filename, param1, NULL);
        if (ret)
        {
            SL_ERR("<sl_secbase_tpm_seal> Failed to run external program! ret:%d\n", ret);
            return SL_EINTERNAL;
        }
    }

    // On success
    return SL_SUCCESS;
}

SL_STATUS _do_sl_secbase_tpm_get_quote(struct tpm_quote_nonce* nonce, uint32_t sb_img_pcr_idx, uint32_t sb_boot_state_pcr_idx)
{
    int ret = 0;
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    char nonce_base64[TPM_QUOTE_NONCE_BASE64_MAX_LENGTH] = {0};

    char param1[PARAM_PCR_LENGTH] = {0};
    char param2[PARAM_PCR_LENGTH] = {0};
    
    // Check: params must be valid
    if(!nonce)
        return SL_EINVAL;

    ret = base64_encode((uint8_t*)nonce, sizeof(struct tpm_quote_nonce), nonce_base64);
    if(!ret || ret > TPM_QUOTE_NONCE_BASE64_MAX_LENGTH)
    {
        SL_ERR("<sl_secbase_tpm_get_quote> Encoding nonce error! ret:%d\n", ret);
        return SL_EINTERNAL;
    }

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
    {
        SL_ERR("<sl_secbase_tpm_get_quote> SecAgent is not loaded. Exit!\n");
        return SL_EINVAL;
    }

    // Check: <sb_img_pcr_idx> and <sb_boot_state_pcr_idx> must be valid PCR index.
    if(!tpm_valid_pcr_index(sb_img_pcr_idx) || !tpm_valid_pcr_index(sb_boot_state_pcr_idx))
    {
        SL_ERR("<sl_secbase_tpm_get_quote> Invalid TPM PCRs (SecBase image PCR:%u, boot state PCR:%u)!\n", sb_img_pcr_idx, sb_boot_state_pcr_idx);
        return SL_EINTERNAL;
    }

    // Generate a new TPM quote
    {
        snprintf(param1, sizeof(param1), "%u", sb_img_pcr_idx);
        snprintf(param2, sizeof(param2), "%u", sb_boot_state_pcr_idx);

        // Replace "=" to be "@" in <nonce_base64> to prevent OS shells from truncating program parameters containing "=".
        // [TODO][Issue 157] Fix the OS shell issue so we do not need to replace the '='.
        replaceEqualsWithAt(nonce_base64);

#if defined(__WINDOWS__)
        const char *exe_filename = "gb_secbase_tpm_quote.bat";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xd1, 0x74, 0x64, 0xc7, 0xc5, 0x56, 0x0c, 0x95,
                0xe0, 0x49, 0x40, 0x67, 0x3f, 0x6b, 0x39, 0x74,
                0x26, 0xfb, 0x26, 0x3d, 0x35, 0xe7, 0x9c, 0x1c,
                0xdb, 0x5e, 0x1b, 0x11, 0xc1, 0xd6, 0x4b, 0xff
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_quote.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xd0, 0xa0, 0xbc, 0xb3, 0x7f, 0xd7, 0xeb, 0x4d,
                0x6c, 0xf9, 0xc7, 0xda, 0xe7, 0xf8, 0x74, 0xdb,
                0x10, 0xcf, 0xd5, 0x69, 0x67, 0x8b, 0x37, 0x05,
                0x6b, 0x4d, 0x81, 0x1c, 0x55, 0x8d, 0xce, 0x04
            }
        };
#endif // __WINDOWS__
        int ret = 0;

        // Run the script to prepare GreenBox.
        strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(exe_filepath, exe_filename);

        ret = os_execute_program_wait_exit(true, exe_filepath, &_file_script_sha256, exe_filename, param1, param2, nonce_base64, NULL);
        if (ret)
        {
            SL_ERR("<sl_secbase_tpm_get_quote> Failed to run external program! ret:%d\n", ret);
            return SL_EINTERNAL;
        }
    }
    
    // On success
    return SL_SUCCESS;
}

SL_STATUS _do_sl_secbase_tpm_get_pcrs(uint32_t sb_img_pcr_idx, uint32_t sb_boot_state_pcr_idx)
{
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    char param1[PARAM_PCR_LENGTH] = {0};
    char param2[PARAM_PCR_LENGTH] = {0};

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
    {
        SL_ERR("<sl_secbase_tpm_get_pcrs> SecAgent is not loaded. Exit!\n");
        return SL_EINVAL;
    }

    // Check: <sb_img_pcr_idx> and <sb_boot_state_pcr_idx> must be valid PCR index.
    if(!tpm_valid_pcr_index(sb_img_pcr_idx) || !tpm_valid_pcr_index(sb_boot_state_pcr_idx))
    {
        SL_ERR("<sl_secbase_tpm_get_pcrs> Invalid TPM PCRs (SecBase image PCR:%u, boot state PCR:%u)!\n", sb_img_pcr_idx, sb_boot_state_pcr_idx);
        return SL_EINTERNAL;
    }

    // Save TPM PCRs of SecBase to the file
    {
        sprintf(param1, "%u", sb_img_pcr_idx);
        sprintf(param2, "%u", sb_boot_state_pcr_idx);

#if defined(__WINDOWS__)
        const char *exe_filename = "gb_secbase_tpm_pcrs.bat";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0x92, 0x90, 0x67, 0xa4, 0x9a, 0x6e, 0x32, 0x20,
                0x92, 0xfd, 0x93, 0xf8, 0xb9, 0xd5, 0x0f, 0x07,
                0x95, 0x4e, 0xd7, 0x68, 0xfe, 0xa1, 0x71, 0x6d,
                0x95, 0xc4, 0x21, 0xd1, 0x4a, 0x60, 0xac, 0xa5
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_pcrs.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xae, 0x36, 0x3b, 0x34, 0x5c, 0x24, 0x2a, 0x50,
                0x83, 0x5f, 0x09, 0x66, 0x24, 0xfe, 0xf9, 0x02,
                0x11, 0x9d, 0x6e, 0xf2, 0x74, 0xa7, 0x5f, 0x8a,
                0xc3, 0xaa, 0xcc, 0xea, 0x61, 0x99, 0xc7, 0xa8
            }
        };
#endif // __WINDOWS__
        int ret = 0;

        // Run the script to prepare GreenBox.
        strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(exe_filepath, exe_filename);
        ret = os_execute_program_wait_exit(true, exe_filepath, &_file_script_sha256, exe_filename, param1, param2, NULL);
        if (ret)
        {
            SL_ERR("<sl_secbase_tpm_get_pcrs> Failed to run external program! ret:%d\n", ret);
            return SL_EINTERNAL;
        }
    }

    // On success
    return SL_SUCCESS;
}

SL_STATUS _do_sl_secbase_tpm_seal(void)
{
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char* param1 = "empty";

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
    {
        SL_ERR("<sl_secbase_tpm_seal> SecAgent is not loaded. Exit!\n");
        return SL_EINVAL;
    }

    // Invoke gb_tpm.exe to seal SecBase's secrets and save the output file alongside UXMHF64 files
    {

#if defined(__WINDOWS__)
        const char *exe_filename = "gb_secbase_tpm_seal.bat";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0x10, 0x3b, 0x1c, 0xec, 0x07, 0x5b, 0x73, 0x7b,
                0x56, 0xf6, 0xeb, 0x33, 0x6c, 0x5f, 0x39, 0x7b,
                0x79, 0x5e, 0x65, 0x22, 0x66, 0xfe, 0x35, 0x30,
                0xf4, 0x7e, 0x6e, 0x65, 0x9d, 0xb7, 0xac, 0xfe
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_seal.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xa1, 0xe3, 0x98, 0x9c, 0x96, 0x02, 0x1d, 0x9c,
                0x9c, 0x9e, 0x00, 0xb9, 0xd3, 0xed, 0x68, 0x91,
                0x0c, 0x01, 0xd9, 0x3b, 0x76, 0xb0, 0xbf, 0xf0,
                0x39, 0x61, 0xd1, 0x3c, 0x6b, 0xc1, 0x9f, 0x14
            }
        };
#endif // __WINDOWS__
        int ret = 0;

        // Run the script to prepare GreenBox.
        strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(exe_filepath, exe_filename);
        ret = os_execute_program_wait_exit(true, exe_filepath, &_file_script_sha256, exe_filename, param1, NULL);
        if (ret)
        {
            SL_ERR("<sl_secbase_tpm_seal> Failed to run external program! ret:%d\n", ret);
            return SL_EINTERNAL;
        }
    }

    // On success
    return SL_SUCCESS;
}

SL_STATUS _do_sl_os_efi_tpm2_event_logs_get_trace(uint32_t sb_img_pcr_idx)
{
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    char param1[PARAM_PCR_LENGTH] = {0}; // <pcr>
    char evl_trace_filepath[FS_MAX_PATH_BUFFER_LEN] = {0}; // <dest. file>

    // Check: <sb_img_pcr_idx> must be valid PCR index.
    if(!tpm_valid_pcr_index(sb_img_pcr_idx))
    {
        SL_ERR("<sl_os_efi_tpm2_event_logs_get_trace> Invalid TPM PCRs (SecBase image PCR:%u)!\n", sb_img_pcr_idx);
        return SL_EINTERNAL;
    }

    // Save TPM PCRs of SecBase to the file
    {
        const char *evl_trace_filename = "../tools/tpmtools/evl_trace.blob";

        sprintf(param1, "%u", sb_img_pcr_idx);

        strncpy(evl_trace_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(evl_trace_filepath, evl_trace_filename);

#if defined(__WINDOWS__)
        const char *exe_filename = "../tools/tpmtools/gb_tpm.exe";
#else
        const char *exe_filename = "../tools/tpmtools/gb_tpm";
#endif // __WINDOWS__
        int ret = 0;

        // Run the script.
        strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(exe_filepath, exe_filename);
        ret = os_execute_program_wait_exit(true, exe_filepath, &_file_gb_tpm_sha256, exe_filename, "--efi-tpm2-event-logs", param1, evl_trace_filepath, NULL);
        if (ret)
        {
            SL_ERR("<sl_os_efi_tpm2_event_logs_get_trace> Failed to run external program! ret:%d\n", ret);
            return SL_EINTERNAL;
        }
    }

    // On success
    return SL_SUCCESS;
}