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

#ifndef TOOL_TPM_CMDS_H
#define TOOL_TPM_CMDS_H

/************ Necessary wolfTPM HAL Definitions **********/
#define TPM2_IoCb NULL

#ifndef __ASSEMBLY__
#include <os/os_headers.h>
#include <app_types.h>
#include <secbase_hypercalls.h>
#include <public/tpmtool_header.h>

#define UXMHF64SealedObjPath		"uxmhfslo.dat"

/// @brief Handle of the TPM owner hierarchy's persistent SRK. Must be the same with UXMHF64.
#define UXMHF64_SRK_PERSISTED_HANDLE (0x81015241)

/// @brief Handle of the physical TPM's persistent AIK. Must be the same with UXMHF64.
#define UXMHF64_AIK_PERSISTED_HANDLE (0x81015217)

int tpm2_dev_init(void);
void tpm2_dev_fini(void);

extern int secbase_get_tpm_pcrs(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT* out_result);

/// @brief Create a new Storeage Root Key (SRK) and persist it at <UXMHF64_SRK_PERSISTED_HANDLE>.
/// @param  
/// @return 
extern int tpm2_srk_create(void);

/// @brief Create a new AIK.
/// @param  
/// @return 
extern int tpm2_aik_create(void);

extern int tpm2_quote_pcr(uint32_t pcr1, uint32_t pcr2, struct tpm_quote_nonce* nonce);

extern int tpm2_pcr_read(uint32_t pcr, uint8_t* value);

extern int tpm2_save_mhv_pcrs_to_file(uint32_t pcr1, uint32_t pcr2);

extern int cmd_secbase_secrets_seal(const char *filepath);
extern int cmd_secbase_secrets_remove(const char *filepath);

/// @brief Parse UEFI TPM2 events
/// @return 
extern int cmd_efi_tpm2_event_logs_get_pcr_trace(uint32_t pcr_idx, SECBASE_TPM_EVENT_LOG_TRACE* out_trace);

/// @brief Return the length of non-empty trace; i.e., the useful info in <trace> without trailing 0s.
/// @param trace 
/// @return 
extern uint32_t cmd_efi_tpm2_event_logs_get_pcr_trace_length(SECBASE_TPM_EVENT_LOG_TRACE* trace);

#endif // __ASSEMBLY__
#endif // TOOL_TPM_CMDS_H