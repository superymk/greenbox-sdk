// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

extern "C"
{
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sl_headers.h>
#include <app_sa_intfs.h>
#include <app_loader.h>
#include <cJSON.h>

#include <dev/usb.h>

#include <ipc/sl_rpc/sl_rpc_protocol.h>
}




/********* Public C Functions *********/

#define MAX_THREADS                 (8192)

extern "C"
{
    extern char* apploader_get_exe_directory();
    
    /// @brief 
    /// @param status 
    /// @param out_resp_data_json Pointer to the response data field in the JSON response.
    /// @return 
    static cJSON* cmd_resp_set_status(SL_STATUS status, cJSON** out_resp_data_json)
    {
        cJSON* result = NULL;
        cJSON* status_json = NULL;
        cJSON* resp_data_json = NULL;

        result = cJSON_CreateObject();
        if(!result)
            goto err;

        // Serialize <status>
        status_json = cJSON_CreateNumber((uint32_t)status);
        if (status_json == NULL)
            goto err;

        cJSON_AddItemToObject(result, "status", status_json);

        // Serialize <resp_data>
        resp_data_json = cJSON_CreateObject();
        if(!resp_data_json)
            goto err;
        cJSON_AddItemToObject(result, "resp_data", resp_data_json);

        if(out_resp_data_json)
            *out_resp_data_json = resp_data_json;

        // On success
        return result;

    err:
        if(result)
            cJSON_Delete(result);
        if(resp_data_json)
            cJSON_Delete(resp_data_json);
        return NULL;
    }
    
    static int cmd_handler(char* request_str, char* response_str)
    {
        int ret = 0;
        cJSON *request_json = NULL;
        cJSON *cmd_json = NULL;
        cJSON *arg1_json = NULL;
        cJSON *arg2_json = NULL;
        cJSON *arg3_json = NULL;
        uint32_t cmd = 0;
        uint32_t arg1 = 0, arg2 = 0, arg3 = 0;

        // Check: Parameters must be valid
        if(!request_str || !response_str)
            return -1;

        request_json = cJSON_Parse(request_str);
        if(!request_json)
            return -1;

        // Parse CMD and args
        cmd_json = cJSON_GetObjectItemCaseSensitive(request_json, "cmd");
        if (cJSON_IsNumber(cmd_json))
            cmd = (uint32_t)cmd_json->valuedouble;
        else
            cmd = 0;

        arg1_json = cJSON_GetObjectItemCaseSensitive(request_json, "arg1");
        if (cJSON_IsNumber(arg1_json))
            arg1 = (uint32_t)arg1_json->valuedouble;
        else
            arg1 = 0;
        (void)arg1;

        arg2_json = cJSON_GetObjectItemCaseSensitive(request_json, "arg2");
        if (cJSON_IsNumber(arg2_json))
            arg2 = (uint32_t)arg2_json->valuedouble;
        else
            arg2 = 0;
        (void)arg2;

        arg3_json = cJSON_GetObjectItemCaseSensitive(request_json, "arg3");
        if (cJSON_IsNumber(arg3_json))
            arg3 = (uint32_t)arg3_json->valuedouble;
        else
            arg3 = 0;
        (void)arg3;

        switch(cmd)
        {
            case SL_SERVER_CMD_sl_ping:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = SL_SUCCESS;

                if(arg1 != SL_PING_REQUEST)
                    status = SL_EINVAL;

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_ping>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_init:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = sl_init();

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_init>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = sl_load_and_run_secos_and_secsrvs();

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = sl_terminate_and_unload_secos_and_secsrvs();

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_dump_dmesg:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = sl_dump_dmesg();

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_dump_dmesg>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_fini:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = sl_fini();

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_fini>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_secprocesses_soft_terminate:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = sl_secprocesses_soft_terminate((spid_t)arg1);

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_secprocesses_soft_terminate>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_secprocesses_get_ids_and_names:
            {
                cJSON* result_json = NULL;
                cJSON* resp_data_json = NULL;
                struct sp_info_id_name_resp resp;
                SL_STATUS status;

                status = sl_secprocesses_get_ids_and_names(&resp);

                result_json = cmd_resp_set_status(status, &resp_data_json);
                if(result_json)
                {
                    // Return result data array
                    cJSON* info_json = NULL;
                    char* result_str = NULL;

                    info_json = cJSON_CreateArray();
                    if(!info_json)
                        goto SL_SERVER_CMD_sl_secprocesses_get_ids_and_names_print_json;
                    cJSON_AddItemToObject(resp_data_json, "info", info_json);

                    {
                        uint32_t i = 0;

                        FOREACH_S(i, resp.num_info, SP_INFO_GET_IDS_NAMES_MAX_RESULTS, 0, 1)
                        {
                            spid_t  sp_id = resp.info[i].sp_id;
                            char* 	sp_name = resp.info[i].sp_name;
                            cJSON* sp_id_json = NULL;
                            cJSON* sp_name_json = NULL;
                            cJSON* info_entry_json = NULL;

                            info_entry_json = cJSON_CreateObject();
                            if(!info_entry_json)
                                goto SL_SERVER_CMD_sl_secprocesses_get_ids_and_names_print_json;

                            cJSON_AddItemToArray(info_json, info_entry_json);

                            sp_id_json = cJSON_CreateNumber((uint32_t)sp_id);
                            if (sp_id_json == NULL)
                                goto SL_SERVER_CMD_sl_secprocesses_get_ids_and_names_print_json;

                            sp_name_json = cJSON_CreateString(sp_name);
                            if (sp_name_json == NULL)
                                goto SL_SERVER_CMD_sl_secprocesses_get_ids_and_names_print_json;

                            cJSON_AddItemToObject(info_entry_json, "sp_id", sp_id_json);
                            cJSON_AddItemToObject(info_entry_json, "sp_name", sp_name_json);
                        }
                    }

SL_SERVER_CMD_sl_secprocesses_get_ids_and_names_print_json:
                    result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    // [TODO][Issue 113] Possible memory leak if not deleting child json nodes
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_secprocesses_get_ids_and_names>\n");
                    ret = -1;
                }

                break;
            }

            case SL_SERVER_CMD_sl_get_dtm_status:
            {
                cJSON* result_json = NULL;
                cJSON* resp_data_json = NULL;
                SL_STATUS status;
                OSSTATUS sa_status;
                uint64_t dtm_status = 0;
                
                sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DTM_SECPROCESS_IS_EXIT, &dtm_status);
                if(SA_FAIL(sa_status))
                    status = SL_EINTERNAL;
                else
                    status = SL_SUCCESS;

                result_json = cmd_resp_set_status(status, &resp_data_json);
                if(result_json)
                {
                    char* result_str = NULL;
                    cJSON* retval1_json = NULL;

                    retval1_json = cJSON_CreateNumber((uint64_t)dtm_status);
                    if(!retval1_json)
                        goto SL_SERVER_CMD_sl_get_dtm_status_create_json_out;
                        
                    cJSON_AddItemToObject(resp_data_json, "ret_val1", retval1_json);

SL_SERVER_CMD_sl_get_dtm_status_create_json_out:
                    result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    // [TODO][Issue 113] Possible memory leak if not deleting child json nodes
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_get_dtm_status>\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_sl_mark_tasks_after_secos_unload:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = SL_SUCCESS;
                uint32_t task_cmd = (uint32_t)arg1;

                status = sl_task_run_after_unload_secos_set(task_cmd, true);

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_sl_mark_tasks_after_secos_unload>\n");
                    ret = -1;
                }

                break;
            }


            // case SL_SERVER_CMD_sl_dev_usb_hc_reset:
            // {
            //     cJSON* result_json = NULL;
            //     SL_CFG* sos_cfg = NULL;
            //     SL_STATUS status = SL_EINVAL;

            //     // Reset the USB bus recorded in <sos_cfg>
            //     sos_cfg = sl_get_sos_cfg();
            //     if(sos_cfg)
            //     {
            //         if(sos_cfg->need_iosm_usb3)
            //         {
            //             status = usb_hc_reset((uint32_t)sos_cfg->usb3_info.hid_dev1_bus_id);
            //         }
            //     }

            //     result_json = cmd_resp_set_status(status, NULL);
            //     if(result_json)
            //     {
            //         char* result_str = cJSON_Print(result_json);
            //         snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
            //         cJSON_Delete(result_json);
            //     }
            //     else
            //     {
            //         printf("Failed to generate response for command <SL_SERVER_CMD_sl_dev_usb_hc_reset>\n");
            //         ret = -1;
            //     }

            //     break;
            // }

            case SL_SERVER_CMD_secbase_tpm_create_keys:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = SL_SUCCESS;

                status = sl_secbase_tpm_create_keys();

                result_json = cmd_resp_set_status(status, NULL);
                if(result_json)
                {
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("Failed to generate response for command <SL_SERVER_CMD_secbase_tpm_create_keys>\n");
                    ret = -1;
                }

                break;
            }

            case SL_SERVER_CMD_secbase_tpm_get_quote:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = SL_SUCCESS;

                uint32_t pcr_mhv_img = (uint32_t)arg1;
                uint32_t pcr_mhv_boot_state = (uint32_t)arg2;

                cJSON *req_nonce_str_json = NULL;
                cJSON *req_nonce_timestamp_json = NULL;
                struct tpm_quote_nonce nonce = {0};
                
                cJSON* resp_data_json = NULL;

                // Get the nonce string from the request
                req_nonce_str_json = cJSON_GetObjectItemCaseSensitive(request_json, "quote_nonce_str");
                if (!cJSON_IsString(req_nonce_str_json))
                {
                    printf("[SL_SERVER_CMD_secbase_tpm_get_quote] Invalid nonce string!\n");
                    status = SL_EINTERNAL;
                    goto SL_SERVER_CMD_secbase_tpm_get_quote_return_status;
                }

                req_nonce_timestamp_json = cJSON_GetObjectItemCaseSensitive(request_json, "quote_nonce_timestamp");
                if (!cJSON_IsNumber(req_nonce_timestamp_json))
                {
                    printf("[SL_SERVER_CMD_secbase_tpm_get_quote] Invalid nonce timestamp!\n");
                    status = SL_EINTERNAL;
                    goto SL_SERVER_CMD_secbase_tpm_get_quote_return_status;
                }

                // Generate the nonce: concatination of the nonce string with the timestamp
                strncpy(nonce.nonce_str, req_nonce_str_json->valuestring, TPM_QUOTE_NONCE_STRING_MAX_LENGTH);
                *(uint64_t*)nonce.freshness = (uint64_t)req_nonce_timestamp_json->valuedouble;

                status = sl_secbase_tpm_get_quote(&nonce, pcr_mhv_img, pcr_mhv_boot_state);
                if (SL_FAIL(status))
                {
                    printf("[SL_SERVER_CMD_secbase_tpm_get_quote] Failed to get TPM Quote!\n");
                    status = SL_EINTERNAL;
                    goto SL_SERVER_CMD_secbase_tpm_get_quote_return_status;
                }

SL_SERVER_CMD_secbase_tpm_get_quote_return_status:
                result_json = cmd_resp_set_status(status, &resp_data_json);
                if(result_json)
                {
                    char quote_files_path[FS_MAX_PATH_BUFFER_LEN] = {0};
                    const char *quote_blob_dirname = "../userdata/tools/tpmtools/";
                    cJSON* ret_quote_files_path_json = NULL;

                    // Get the absolute file path of generated TPM quote files
                    strncpy(quote_files_path, apploader_get_exe_directory(), MAX_PATH_LEN);
                    strcat(quote_files_path, quote_blob_dirname);

                    ret_quote_files_path_json = cJSON_CreateString(quote_files_path);
                    if(!ret_quote_files_path_json)
                        goto SL_SERVER_CMD_secbase_tpm_get_quote_create_json_out;
                        
                    cJSON_AddItemToObject(resp_data_json, "ret_tpm_quote_files_path", ret_quote_files_path_json);

SL_SERVER_CMD_secbase_tpm_get_quote_create_json_out:
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("[SL_SERVER_CMD_secbase_tpm_get_quote] Failed to generate response\n");
                    ret = -1;
                }

                break;
            }

            case SL_SERVER_CMD_secbase_tpm_get_pcrs:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = SL_SUCCESS;

                uint32_t pcr_mhv_img = (uint32_t)arg1;
                uint32_t pcr_mhv_boot_state = (uint32_t)arg2;

                cJSON* resp_data_json = NULL;

                status = sl_secbase_tpm_get_pcrs(pcr_mhv_img, pcr_mhv_boot_state);
                if (SL_FAIL(status))
                {
                    printf("[SL_SERVER_CMD_secbase_tpm_get_pcrs] Failed to get TPM PCRs!\n");
                    status = SL_EINTERNAL;
                    goto SL_SERVER_CMD_secbase_tpm_get_pcrs_return_status;
                }

SL_SERVER_CMD_secbase_tpm_get_pcrs_return_status:
                result_json = cmd_resp_set_status(status, &resp_data_json);
                if(result_json)
                {
                    char pcrs_files_path[FS_MAX_PATH_BUFFER_LEN] = {0};
                    const char *pcrs_blob_dirname = "../tools/tpmtools/";
                    cJSON* ret_pcrs_files_path_json = NULL;

                    // Get the absolute file path of generated TPM pcrs files
                    strncpy(pcrs_files_path, apploader_get_exe_directory(), MAX_PATH_LEN);
                    strcat(pcrs_files_path, pcrs_blob_dirname);

                    ret_pcrs_files_path_json = cJSON_CreateString(pcrs_files_path);
                    if(!ret_pcrs_files_path_json)
                        goto SL_SERVER_CMD_secbase_tpm_get_pcrs_create_json_out;
                        
                    cJSON_AddItemToObject(resp_data_json, "ret_tpm_pcrs_files_path", ret_pcrs_files_path_json);

SL_SERVER_CMD_secbase_tpm_get_pcrs_create_json_out:
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("[SL_SERVER_CMD_secbase_tpm_get_pcrs] Failed to generate response\n");
                    ret = -1;
                }

                break;
            }
            case SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace:
            {
                cJSON* result_json = NULL;
                SL_STATUS status = SL_SUCCESS;

                uint32_t pcr_mhv_img = (uint32_t)arg1;

                cJSON* resp_data_json = NULL;

                status = sl_os_efi_tpm2_event_logs_get_trace(pcr_mhv_img);
                if (SL_FAIL(status))
                {
                    printf("[SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace] Failed to get EFI event log trace from the untrusted OS!\n");
                    status = SL_EINTERNAL;
                    goto SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace_return_status;
                }

SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace_return_status:
                result_json = cmd_resp_set_status(status, &resp_data_json);
                if(result_json)
                {
                    char evl_trace_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};
                    const char *evl_trace_file_dirname = "../tools/tpmtools/";
                    cJSON* ret_evl_trace_path_json = NULL;

                    // Get the absolute file path of generated TPM pcrs files
                    strncpy(evl_trace_file_path, apploader_get_exe_directory(), MAX_PATH_LEN);
                    strcat(evl_trace_file_path, evl_trace_file_dirname);

                    ret_evl_trace_path_json = cJSON_CreateString(evl_trace_file_path);
                    if(!ret_evl_trace_path_json)
                        goto SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace_create_json_out;
                        
                    cJSON_AddItemToObject(resp_data_json, "ret_evl_trace_file_path", ret_evl_trace_path_json);

SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace_create_json_out:
                    char* result_str = cJSON_Print(result_json);
                    if(result_str)
                    {
                        snprintf(response_str, MAX_JSON_MSG_SIZE, "%s", result_str);
                        cJSON_free(result_str);
                    }
                    else
                    {
                        // Send empty response in JSON if <result_str> is invalid.
                        strncpy(response_str, "{}", MAX_JSON_MSG_SIZE);
                    }
                    cJSON_Delete(result_json);
                }
                else
                {
                    printf("[SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace] Failed to generate response\n");
                    ret = -1;
                }

                break;
            }

            default:
            {
                printf("Invalid CMD %u\n", cmd);
                ret = -1;
                break;
            }
        }

        cJSON_Delete(request_json);
        return ret;
    }
}

extern "C"
{
    static void *connection_handler(void *);

    static bool RUN_SERVER = true;

    /// @brief 
    /// @param  
    /// @return On success, a file descriptor for the new socket is returned. On error, -1 is returned.
    static int server_socket_bind(void)
    {
        int server_socket;
        struct sockaddr_in server_addr;

        server_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (server_socket == -1)
        {
            perror("Socket initialisation failed");
            return -1;
        }
        else
            printf("Server socket created successfully\n");

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SL_SERVER_PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        // bind the socket to the specified IP addr and port
        if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
        {
            printf("socket bind failed...\n");
            return -1;
        }
        else
            printf("Socket successfully binded..\n");

        // On success
        return server_socket;
    }

    /// @brief 
    /// @param server_socket 
    /// @return On success, return 0. On error, return -1.
    static int server_socket_listen(int server_socket)
    {
        if (listen(server_socket, 3) != 0)
        {
            printf("Listen failed...\n");
            return -1;
        }

        {
            int k = 0;
            int num_threads = 0;
            pthread_t threads[MAX_THREADS];
            while (RUN_SERVER && (num_threads < MAX_THREADS))
            {
                printf("Server listening...\n");
                int client_socket = accept(server_socket, NULL, NULL);
                puts("Connection accepted");
                if (pthread_create(&threads[num_threads], NULL, connection_handler, &client_socket) < 0)
                {
                    perror("Could not create thread");
                    return -1;
                }
                if (client_socket < 0)
                {
                    printf("server acccept failed...\n");
                    return -1;
                }
                else
                    printf("Server acccept the client...\n");
                puts("Handler assigned");
                num_threads++;
            }

            for (k = 0; k < MAX_THREADS; k++)
            {
                pthread_join(threads[k], NULL);
            }
        }

        // int send_status;
        // send_status=send(client_socket, server_message, sizeof(server_message), 0);
        close(server_socket);

        return 0;
    }

    void *connection_handler(void *client_socket)
    {
        int socket = *(int *)client_socket;
        int read_len;
        int send_status;
        char client_req_str[MAX_JSON_MSG_SIZE];
        char client_resp_str[MAX_JSON_MSG_SIZE];
        
        while ( RUN_SERVER)
        {
            read_len = recv(socket, client_req_str, MAX_JSON_MSG_SIZE, 0);
            if(read_len > 0)
            {
                // end of string marker
                client_req_str[read_len] = '\0';

                // Handle commands
                cmd_handler(client_req_str, client_resp_str);

                // Send the message back to client
                send_status = send(socket, client_resp_str, strnlen(client_resp_str, MAX_JSON_MSG_SIZE), 0);
                (void)send_status;
            }
            else if (read_len == 0)
            {
                // Client disconnected
                printf("Client disconnected\n");
                break;
            }
            else
            {
                // recv error
                perror("recv failed");
                break;
            }
        }

        close(socket);
        return 0;
    }

    void sl_main_clean_on_exception(void)
    {
        printf("Server encounters exception!\n");
        RUN_SERVER = false;

        sl_log_fini();
    }

    int main(int argc, char **argv)
    {
        SL_STATUS status = SL_SUCCESS;
        int ret = 0;
        int server_socket = 0;

        // Init log sub-system.
        status = sl_log_init();
        if(SL_FAIL(status))
		{
			printf("Initialize log error! status:%lu\n", status);
            goto err;
		}
        // SOSLoader can use print macros (e.g., SL_DEBUG, SL_INFO, SL_ERR) from now on.

        // Ensure only one server can run and bind the port
        ret = server_socket_bind();
        if (ret == -1)
        {
            printf("socket bind error\n");
            goto err;
        }
        server_socket = ret;

        // Run scripts to prepare GreenBox
        status = sl_run_prepare_script();
        if(SL_FAIL(status))
		{
			printf("Run GreenBox prepare script error! status:%lu\n", status);
            goto err;
		}

        // Listen on the socket. Ready to take GreenBox loader's requests
        ret = server_socket_listen(server_socket);
        if (ret)
        {
            printf("socket listen error\n");
            goto err;
        }

        sl_log_fini();

        return 0;

    err:
        sl_log_fini();
        exit(1);
    }



    // int main()
    // {
    //     SL_STATUS status = SL_SUCCESS;

    //     // char quote_nonce_str[TPM_QUOTE_NONCE_STRING_MAX_LENGTH] = {0};
    //     char* quote_nonce_str = "abcdef";

    //     // Run scripts to prepare GreenBox
    //     status = sl_run_prepare_script();
    //     if(SL_FAIL(status))
	// 	{
	// 		printf("Run GreenBox prepare script error! status:%lu\n", status);
    //         exit(1);
	// 	}

    //     status = sl_secbase_tpm_get_quote(quote_nonce_str);
    //     if (SL_FAIL(status))
    //     {
    //         printf("[SL_SERVER_CMD_secbase_tpm_get_quote] Failed to get TPM Quote!\n");
    //         return SL_EINTERNAL;
    //     }
    // }



// extern SL_STATUS sl_load_and_run_secos_and_secsrvs_test(void);

//     int main()
//     {
//         sl_load_and_run_secos_and_secsrvs_test();
//     }




// extern void sl_sos_minimal_load_and_run(void);

//     int main()
//     {
//         SL_STATUS status = SL_SUCCESS;

//         // Run scripts to prepare GreenBox
//         status = sl_run_prepare_script();
//         if(SL_FAIL(status))
// 		{
// 			printf("Run GreenBox prepare script error! status:%lu\n", status);
//             exit(1);
// 		}


//         sl_sos_minimal_load_and_run();
//     }



}