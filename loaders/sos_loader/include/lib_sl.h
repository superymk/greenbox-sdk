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

#ifndef SOS_LD_API_H
#define SOS_LD_API_H

#include <app_types.h>
#include <sl_status.h>
#include <sl_cfg_reader.h>
#include <tpmtool/include/tpmtool_header.h>

#ifndef __ASSEMBLY__

/// @brief Run the GreenBox prepare script that are necessary for <sl_init>; e.g., loading the SecAgent.
/// @param
/// @return
extern SL_STATUS sl_run_prepare_script(void);

/// @brief Initialize SOSLoader log subsystem. This function may be called once.
/// @param  
/// @return 
extern SL_STATUS sl_log_init(void);

/// @brief 
/// @param  
extern void sl_log_fini(void);

/// @brief Initialize SOSLoader. This function may be called multiple times on-demand.
/// @param  
/// @return 
extern SL_STATUS sl_init(void);
extern SL_STATUS sl_fini(void);

/// @brief Load and run SecOS.
/// @param
/// @return Return SL_EINVAL on resource provisioning errors. If returning other errors, the caller must call
/// <sl_terminate_and_unload_secos_and_secsrvs> to stop SecOS
extern SL_STATUS sl_load_and_run_secos_and_secsrvs(void);
extern SL_STATUS sl_terminate_and_unload_secos_and_secsrvs(void);
extern SL_STATUS sl_dump_dmesg(void);

/// @brief Set a task required to run or not after SecOS is successfully unloaded. 
/// @param task_cmd 
/// @param task_need_run 
/// @return 
extern SL_STATUS sl_task_run_after_unload_secos_set(uint32_t task_cmd, bool task_need_run);

/// @brief Return the SecOS config.
extern SL_CFG *sl_get_sos_cfg(void);

/********* SecProcess *********/
/// @brief 
/// @param sp_id 
/// @return 
extern SL_STATUS sl_secprocesses_soft_terminate(spid_t sp_id);
extern SL_STATUS sl_secprocesses_get_ids_and_names(struct sp_info_id_name_resp *resp_buf);

/********* SecBase TPM Functions *********/
#define tpm_valid_pcr_index(pcr) (((uint32_t)(pcr)) < 24)

/// @brief Create TPM keys required by GreenBox.
/// @param  
/// @return 
extern SL_STATUS sl_secbase_tpm_create_keys(void);

/// @brief Generate TPM quote of SecBase by the physical TPM device.
/// @param nonce 
/// @param pcr_mhv_img 
/// @param pcr_mhv_boot_state 
/// @return 
extern SL_STATUS sl_secbase_tpm_get_quote(struct tpm_quote_nonce* nonce, uint32_t pcr_mhv_img, uint32_t pcr_mhv_boot_state);

/// @brief Return the PCR values of SecBase from the physical TPM device. 
/// @param pcr_mhv_img 
/// @param pcr_mhv_boot_state 
/// @return 
extern SL_STATUS sl_secbase_tpm_get_pcrs(uint32_t pcr_mhv_img, uint32_t pcr_mhv_boot_state);

/********* Untrusted OS TPM Functions *********/
/// @brief Parse EFI TPM2.X event logs and dump all measurement extends of PCR <pcr_mhv_img> in to the file 
/// "tools/tpmtools/evl_trace.blob".
/// @param pcr_mhv_img 
/// @return 
extern SL_STATUS sl_os_efi_tpm2_event_logs_get_trace(uint32_t pcr_mhv_img);

/********* SecAgent Event Callback *********/
// extern void on_secagent_notify(void);

/// @brief SOSLoader executable must provide <sl_main_clean_on_exception> to perform *addtional* finalization on
/// process exceptions
/// @param
extern void sl_main_clean_on_exception(void);

#endif // __ASSEMBLY__
#endif // SOS_LD_API_H