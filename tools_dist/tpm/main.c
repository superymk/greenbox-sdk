// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <log.h>

#include "app_os_if.h"
#include "utils_string.h"
#include "cmds.h"
#include "base64.h"
#include "utils.h"

#define SHA256_HASH_LENGTH  (32)

static NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT pcr = {0};

__attribute__((unused)) static int _tpm_print_pcrs(void)
{
	int ret = 0;
    char pcr_str[2*SHA256_HASH_LENGTH + 1] = {0};

    // lock <dmesg_buf>
	app_lock_mem(&pcr, sizeof(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT));

    ret = secbase_get_tpm_pcrs(&pcr);
    if(ret)
    {
        printf("SecBase TPM Tool Error: Failed to get TPM PCRs from SecBase! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    // Print PCR
    if((pcr.rot_type == SECBASE_ROT_SRTM) || (pcr.rot_type == SECBASE_ROT_DRTM))
    {
        byte_array_to_hex_string(pcr.img_digest, SHA256_HASH_LENGTH, pcr_str);
        printf("SecBase (image) uses root of trust type:%u, PCR: %s\n", pcr.rot_type, pcr_str);
    }
    else
    {
        printf("Warning: SecBase is not using TPM\n");
    }

    // On success
    ret = 0;

out:
    app_unlock_mem(&pcr, sizeof(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT));
	return ret;
}

__attribute__ ((unused)) static void replaceAtWithEquals(char *str) 
{
    while (*str != '\0') {
        if (*str == '@') {
            *str = '=';
        }
        str++;
    }
}

/// @brief Handle the command "--efi-tpm2-event-logs". 
/// @param pcr_idx 
/// @param result_filepath 
/// @return 
static int _cmd_efi_tpm2_event_logs_b64(uint32_t pcr_idx, char* result_filepath)
{
    int ret = 0;
    SECBASE_TPM_EVENT_LOG_TRACE* trace = NULL;
    uint32_t trace_len = 0;
    // char* result = NULL;
    // uint32_t result_len = 0;

    trace = malloc(sizeof(SECBASE_TPM_EVENT_LOG_TRACE));
    if(!trace)
    {
        printf("SecBase TPM Tool: Failed to allocate the result memory for EFI event log trace! ret:%d\n", ret);
        ret = 1;
        goto out;
    }
    memset(trace, 0, sizeof(SECBASE_TPM_EVENT_LOG_TRACE));

// #define SECBASE_TPM_EVENT_LOG_TRACE_BASE64_LENGTH   (2 * sizeof(SECBASE_TPM_EVENT_LOG_TRACE))
//     result = malloc(SECBASE_TPM_EVENT_LOG_TRACE_BASE64_LENGTH);
//     if(!result)
//     {
//         printf("SecBase TPM Tool: Failed to allocate the result memory for EFI event log trace! ret:%d\n", ret);
//         ret = 1;
//         goto out;
//     }
//     memset(result, 0, SECBASE_TPM_EVENT_LOG_TRACE_BASE64_LENGTH);

    ret = cmd_efi_tpm2_event_logs_get_pcr_trace(pcr_idx, trace);
    if (ret)
    {
        printf("SecBase TPM Tool: Failed to parse UEFI TPM2 event logs! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    trace_len = cmd_efi_tpm2_event_logs_get_pcr_trace_length(trace);
    if(!trace_len)
    {
        printf("SecBase TPM Tool: Invalid length of parsed UEFI TPM2 event logs! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    // // Encode the result
    // result_len = base64_encode((uint8_t*)trace, trace_len, result);
    // if (result_len > SECBASE_TPM_EVENT_LOG_TRACE_BASE64_LENGTH)
    // {
    //     printf("SecBase TPM Tool: Failed to encode UEFI TPM2 event log trace! result_len:%u\n", result_len);
    //     ret = 1;
    //     goto out;
    // }

    // Save the result
    ret = fs_create_directories(result_filepath);
    if (ret)
    {
        printf("SecBase TPM Tool: Create directory for UEFI TPM2 event logs parse result error! ret: 0x%x\n", ret);
        ret = 1;
        goto out;
    }

    // ret = fs_write_file(result_filepath, (uint8_t*)result, result_len);
    ret = fs_write_file(result_filepath, (uint8_t*)trace, trace_len);
    if (ret)
    {
        printf("SecBase TPM Tool: Write UEFI TPM2 event logs parse result file error! ret: 0x%x\n", ret);
        ret = 1;
        goto out;
    }

    // On success
    ret = 0;

out:
    // if(result)
    //     free(result);
    if(trace)
        free(trace);
    return ret;
}




/********* Public Functions *********/
void print_usage(void)
{
    printf("SecBase TPM Tool:\n");
    printf("Author: Miao Yu, Virgil Gligor\n");
    printf("\t -h: help.\n");
    printf("\t -q <pcr1> <pcr2> <nonce>: Generate a quote of the given pair of PCRs by the physical TPM. The nonce must be less than 32 characters.\n");
    printf("\t -qnb64 <pcr1> <pcr2> <nonce-base64>: Generate a quote of the given pair of PCRs by the physical TPM.\n");
    printf("\t -p <pcr1> <pcr2>: Print PCRs.\n");
    printf("\t -pf <pcr1> <pcr2>: Store PCRs to files.\n");
    printf("\t --secbase-create-keys: Create SRK if not exist and create a new AIK and ask SecBase to persist them.\n");
    printf("\t --secbase-seal: Invoke SecBase to seal its secrets and save the output file alongside UXMHF64 files (Scan and use the first UXMHF64 location only).\n");
    printf("\t --secbase-seal-delete: Delete the sealed secrets alongside UXMHF64 files (Scan and use the first UXMHF64 location only).\n");
    printf("\t --efi-tpm2-event-logs <pcr> <file>: Get the trace of PCR measurements from EFI event logs.\n");
}

int main(int argc, char* argv[])
{
    int ret = 0;

    // Init TPM2 library
    ret = tpm2_dev_init();
    if(ret)
    {
        printf("SecBase TPM Tool: Init TPM2 error!\n");
        return 1;
    }

    if (argc >= 2) 
    {
        if(strcmp(argv[1], "-h") == 0)
        {
            print_usage();
        }
        else if(strcmp(argv[1], "-q") == 0)
        {
            // Quote PCRs
            int pcr1 = 0;
            int pcr2 = 0;
            struct tpm_quote_nonce nonce = {0};
            char* nonce_str = NULL;

            if(argc < 5)
            {
                printf("SecBase TPM Tool: Missing arguments\n");
                printf("\t -q <pcr1> <pcr2> <nonce>: Generate a quote of the given pair of PCRs by the physical TPM. The nonce must be less than 32 characters.\n");
                ret = 1;
                goto out; 
            }

            nonce_str = argv[4];

            // Check: Nonce string must be less than TPM_QUOTE_NONCE_STRING_MAX_LENGTH characters.
            if (strnlen(nonce_str, TPM_QUOTE_NONCE_STRING_MAX_LENGTH) >= TPM_QUOTE_NONCE_STRING_MAX_LENGTH)
            {
                ret = 1;
                goto out;
            }

            pcr1 = atoi(argv[2]);
            pcr2 = atoi(argv[3]);
            memcpy(nonce.nonce_str, nonce_str, strnlen(nonce_str, TPM_QUOTE_NONCE_STRING_MAX_LENGTH));

            ret = tpm2_quote_pcr(pcr1, pcr2, &nonce);
            if(ret)
            {
                printf("SecBase TPM Tool: Quote PCR 4 and 15 error!\n");
                ret = 1;
                goto out;
            }
        }
        else if(strcmp(argv[1], "-qnb64") == 0)
        {
            // Quote PCRs
            int pcr1 = 0;
            int pcr2 = 0;
            struct tpm_quote_nonce nonce = {0};
            char* nonce_base64 = NULL;

            if(argc < 5)
            {
                printf("SecBase TPM Tool: Missing arguments!\n");
                printf("\t -qnb64 <pcr1> <pcr2> <nonce-base64>: Generate a quote of the given pair of PCRs by the physical TPM.\n");
                ret = 1;
                goto out; 
            }

            pcr1 = atoi(argv[2]);
            pcr2 = atoi(argv[3]);
            nonce_base64 = argv[4];

            // [TODO][Issue 157] Fix the OS shell issue so we do not need to replace the '='.
            replaceAtWithEquals(nonce_base64);

            // Check: Nonce string must be less than TPM_QUOTE_NONCE_BASE64_MAX_LENGTH characters.
            if (strnlen(nonce_base64, TPM_QUOTE_NONCE_BASE64_MAX_LENGTH) >= TPM_QUOTE_NONCE_BASE64_MAX_LENGTH)
            {
                ret = 1;
                goto out;
            }

            ret = base64_decode(nonce_base64, strnlen(nonce_base64, TPM_QUOTE_NONCE_BASE64_MAX_LENGTH), (uint8_t*)&nonce);
            if(ret != sizeof(struct tpm_quote_nonce))
            {
                printf("SecBase TPM Tool: Malformed parameter for nonce!\n");
                ret = 1;
                goto out;
            }

            ret = tpm2_quote_pcr(pcr1, pcr2, &nonce);
            if(ret)
            {
                printf("SecBase TPM Tool: Quote PCR 4 and 15 error!\n");
                ret = 1;
                goto out;
            }
        }
        else if(strcmp(argv[1], "-p") == 0)
        {
            // Read PCRs
            int pcr1 = 0;
            int pcr2 = 0;
            uint8_t value[32] = {0};

            if(argc < 4)
            {
                printf("SecBase TPM Tool: Missing arguments\n");
                printf("\t -p <pcr1> <pcr2>: Print PCRs.\n");
                ret = 1;
                goto out; 
            }

            pcr1 = atoi(argv[2]);
            pcr2 = atoi(argv[3]);

            ret = tpm2_pcr_read(pcr1, value);
            if(ret)
            {
                printf("SecBase TPM Tool: Read PCR %d error! ret:%d\n", pcr1, ret);
                ret = 1;
                goto out;
            }
            printf("PCR %d:", pcr1);
            print_hex("", value, 32);
            printf("\n");

            ret = tpm2_pcr_read(pcr2, value);
            if(ret)
            {
                printf("SecBase TPM Tool: Read PCR %d error! ret:%d\n", pcr2, ret);
                ret = 1;
                goto out;
            }
            printf("PCR %d:", pcr2);
            print_hex("", value, 32);
        }
        else if(strcmp(argv[1], "-pf") == 0)
        {
            // Save PCR 7 and 15 to the file.
            int pcr1 = 0;
            int pcr2 = 0;

            if(argc < 4)
            {
                printf("SecBase TPM Tool: Missing arguments\n");
                printf("\t -pf <pcr1> <pcr2>: Store PCRs to files.\n");
                ret = 1;
                goto out; 
            }

            pcr1 = atoi(argv[2]);
            pcr2 = atoi(argv[3]);

            ret = tpm2_save_mhv_pcrs_to_file(pcr1, pcr2);
            if(ret)
            {
                printf("SecBase TPM Tool: Save PCR %d and %d to file error! ret:%d\n", pcr1, pcr2, ret);
                ret = 1;
                goto out;
            }
        }
        else if(strcmp(argv[1], "--secbase-create-keys") == 0)
        {
            // Create SRK
            ret = tpm2_srk_create();
            if(ret)
            {
                printf("SecBase TPM Tool: Create SRK error! ret:%d\n", ret);
                ret = 1;
                goto out;
            }

            // Create a new AIK
            ret = tpm2_aik_create();
            if(ret)
            {
                printf("SecBase TPM Tool: Create AIK error! ret:%d\n", ret);
                ret = 1;
                goto out;
            }
        }
        else if(strcmp(argv[1], "--secbase-seal") == 0)
        {
            // Seal SecBase secrets
            char uxmhf64_drive_str[FS_MAX_PATH_BUFFER_LEN] = {0};

            //// Locate the first drive containing UXMHF64
            ret = uxmhf_mhv_locate_first_drive(uxmhf64_drive_str);
            if(ret)
            {
                printf("SecBase TPM Tool: Cannot locate SecBase files! ret:%d\n", ret);
                ret = 1;
                goto out;
            }
        
            strcat(uxmhf64_drive_str, UXMHF64SealedObjPath);

            ret = cmd_secbase_secrets_seal(uxmhf64_drive_str);
            if(ret)
            {
                printf("SecBase TPM Tool: Add sealed secrets error! ret:%d\n", ret);
                ret = 1;
                goto out;
            }
        }
        else if(strcmp(argv[1], "--secbase-seal-delete") == 0)
        {
            // Seal SecBase secrets
            char uxmhf64_drive_str[FS_MAX_PATH_BUFFER_LEN] = {0};

            //// Locate the first drive containing UXMHF64
            ret = uxmhf_mhv_locate_first_drive(uxmhf64_drive_str);
            if(ret)
            {
                printf("SecBase TPM Tool: Cannot locate SecBase files! ret:%d\n", ret);
                ret = 1;
                goto out;
            }
        
            strcat(uxmhf64_drive_str, UXMHF64SealedObjPath);

            ret = cmd_secbase_secrets_remove(uxmhf64_drive_str);
            if(ret)
            {
                printf("SecBase TPM Tool: Remove sealed secrets error! ret:%d\n", ret);
                ret = 1;
                goto out;
            }
        }
        else if(strcmp(argv[1], "--efi-tpm2-event-logs") == 0)
        {
            int pcr = 0;

            if(argc < 4)
            {
                printf("\t --efi-tpm2-event-logs <pcr> <file>: Get the trace of PCR measurements from EFI event logs.\n");
                ret = 1;
                goto out; 
            }

            pcr = atoi(argv[2]);

            ret = _cmd_efi_tpm2_event_logs_b64((uint32_t)pcr, argv[3]);
            if (ret)
            {
                ret = 1;
                goto out;
            }
        }
        else
        {
            // Unknown options
            print_usage();
        }
    }
    else
    {
        // Unknown options
        print_usage();
    }

    // ret = _tpm_print_pcrs();
    // if(ret)
    // {
    //     printf("SecBase TPM Tool: Print PCRs error\n");
    //     ret = 1;
    //     goto out;
    // }

    // ret = _tpm_print_nvram_index();
    // if(ret)
    // {
    //     printf("SecBase TPM Tool: Print NVRAM index error\n");
    //     ret = 1;
    //     goto out;
    // }

    // On success
    ret = 0;

out:
    tpm2_dev_fini();
    return ret;
}