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

#ifndef APP_SOSLoader_COMMUNICATION
#define APP_SOSLoader_COMMUNICATION



#ifndef __ASSEMBLY__

/// @brief Connect to SOSLoader server. Call this function again without <app_sl_rpc_fini> does not create a new 
/// connection.
/// @param  
/// @return 
extern int app_sl_rpc_init(void);

/// @brief Close the connection to SOSLoader server
/// @param  
extern void app_sl_rpc_fini(void);

extern bool app_sl_rpc_is_initialized(void);

/// @brief 
/// @param task_cmd 
/// @param out_sl_status 
/// @return 
extern int app_sl_rpc_call_sl_mark_tasks_after_secos_unload(
    uint32_t task_cmd,
    SL_STATUS* out_sl_status
);

/// @brief 
/// @param in_quote_nonce_str 
/// @param in_quote_nonce_timestamp
/// @param out_sl_status Required. Updated with the result status of SOSLoader server.
/// @param out_tpm_quote_files_dirname Optional. Updated with the absolute path of generated TPM quote files.
/// @return 
extern int app_sl_rpc_call_sl_secbase_tpm_get_quote(
    char* in_quote_nonce_str, 
    uint64_t in_quote_nonce_timestamp, 
    uint32_t pcr_mhv_img, 
    uint32_t pcr_mhv_boot_state,
    SL_STATUS* out_sl_status, 
    char* out_tpm_quote_files_dirname
);

/// @brief 
/// @param pcr_mhv_img 
/// @param pcr_mhv_boot_state 
/// @param out_sl_status Required. Updated with the result status of SOSLoader server.
/// @param out_tpm_pcrs_files_dirname Optional. Updated with the absolute path of generated TPM PCRs files.
/// @return 
extern int app_sl_rpc_call_sl_secbase_tpm_get_pcrs(
    uint32_t pcr_mhv_img, 
    uint32_t pcr_mhv_boot_state,
    SL_STATUS* out_sl_status, 
    char* out_tpm_pcrs_files_dirname
);

/// @brief 
/// @param pcr_mhv_img 
/// @param out_sl_status Required. Updated with the result status of SOSLoader server.
/// @param out_evl_trace_file_dirname Optional. Updated with the absolute path of generated event log trace.
/// @return 
extern int app_sl_rpc_call_sl_os_efi_get_tpm2_event_logs_trace(
    uint32_t pcr_mhv_img, 
    SL_STATUS* out_sl_status, 
    char* out_evl_trace_file_dirname
);


#endif // __ASSEMBLY__
#endif // APP_SOSLoader_COMMUNICATION