// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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
                0xe2, 0xb9, 0x41, 0xfe, 0x47, 0x2b, 0xdc, 0x3f,
                0x46, 0x72, 0x15, 0x29, 0x63, 0xe7, 0xb4, 0xdf,
                0xc4, 0x11, 0xb4, 0x93, 0x83, 0x43, 0xd4, 0x19,
                0xea, 0xb0, 0x6e, 0x68, 0x72, 0xfc, 0x2b, 0x42
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_create_keys.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0x80, 0x2e, 0x09, 0x5e, 0xcf, 0x8c, 0x67, 0xfd,
                0x1a, 0x62, 0x5a, 0xb7, 0x12, 0x8a, 0xe7, 0x8f,
                0xbc, 0x1b, 0x00, 0x50, 0xa8, 0xae, 0xb1, 0xe9,
                0x49, 0x1a, 0x32, 0x0f, 0xb7, 0xd0, 0x5c, 0xd4
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
                0x56, 0xdf, 0xd9, 0x1b, 0xdb, 0x83, 0x41, 0x87,
                0xed, 0x3e, 0x67, 0x0e, 0xd6, 0x51, 0xf4, 0x2d,
                0xf3, 0x5d, 0xb8, 0x9b, 0x55, 0x05, 0xc8, 0xaf,
                0x38, 0xa4, 0x9c, 0x21, 0xf1, 0xa1, 0xa9, 0x5e
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_quote.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0x9a, 0xeb, 0x19, 0xae, 0x80, 0xa8, 0xc6, 0xf9,
                0x97, 0xe7, 0x9b, 0x2b, 0xf0, 0x4d, 0x92, 0x11,
                0xb7, 0x4d, 0x85, 0x7f, 0xd4, 0x17, 0xc0, 0xd1,
                0x62, 0xcc, 0x20, 0x29, 0x79, 0x17, 0x3b, 0x40
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
                0x5c, 0xd0, 0x2e, 0x5e, 0x7d, 0xb9, 0xc9, 0x24,
                0x3a, 0x8d, 0xfd, 0xb6, 0xbd, 0xcd, 0x16, 0xd3,
                0x63, 0x75, 0xc3, 0xa8, 0xcc, 0x54, 0x69, 0x33,
                0xb0, 0xb6, 0x8d, 0x32, 0x9b, 0x83, 0x4b, 0x9e
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_pcrs.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xaf, 0x79, 0xd6, 0x97, 0x7b, 0x8c, 0x64, 0xe9,
                0x35, 0xe1, 0xed, 0x6a, 0x63, 0x92, 0x5d, 0xcb,
                0x64, 0x96, 0xd3, 0x3a, 0x35, 0x8d, 0x0c, 0x59,
                0x05, 0x5f, 0xa1, 0x47, 0xc7, 0x99, 0x6d, 0xcf
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
                0xcf, 0x89, 0x74, 0x77, 0xfa, 0x27, 0x22, 0x66,
                0xe8, 0xeb, 0x35, 0xa1, 0x2f, 0x90, 0xb1, 0x82,
                0x23, 0xf8, 0x99, 0x85, 0xed, 0xed, 0xa3, 0x97,
                0x76, 0x14, 0x0c, 0xeb, 0x93, 0x24, 0xf8, 0xb4
            }
        };
#else
        const char *exe_filename = "gb_secbase_tpm_seal.sh";
        const union sha_digest _file_script_sha256 = {
            .sha2_256_digest = {
                0xe9, 0x4f, 0xee, 0xc2, 0x08, 0xa8, 0x07, 0x0a,
                0xb3, 0x65, 0x07, 0xf4, 0xda, 0x97, 0xf9, 0xae,
                0x1e, 0xf6, 0xd1, 0x92, 0xfa, 0x46, 0x35, 0x12,
                0x24, 0xc5, 0x01, 0xbf, 0x81, 0xe3, 0x92, 0x3a
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