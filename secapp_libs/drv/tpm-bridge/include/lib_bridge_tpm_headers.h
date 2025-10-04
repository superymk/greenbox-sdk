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

/// @file This lib is the HID/GUI-bridge functions of the SecProcess side

#ifndef LIBBRIDGE_HEADERS_H
#define LIBBRIDGE_HEADERS_H

#include <stdio.h>
#include <string.h>
#include <sap_headers.h>
#include <palddk.h>

#include <app_sp_bridge_cmds.h>

#define LIBBRIDGE_TPM_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#define BRIDGE_TPM_DEBUG(format, ...) \
	do{	\
		if((LIBBRIDGE_TPM_LOG >= LOG_DEBUG) && (LIBBRIDGE_TPM_LOG != LOG_NONE)) {  \
		printf("[LibBridge-TPM][Debug] "format, ## __VA_ARGS__); } }while(0)

#define BRIDGE_TPM_INFO(format, ...) \
	do{	\
		if((LIBBRIDGE_TPM_LOG >= LOG_INFO) && (LIBBRIDGE_TPM_LOG != LOG_NONE)) {  \
		printf("[LibBridge-TPM] "format, ## __VA_ARGS__); } }while(0)

#define BRIDGE_TPM_ERR(format, ...) \
	do{	\
		if((LIBBRIDGE_TPM_LOG >= LOG_ERR) && (LIBBRIDGE_TPM_LOG != LOG_NONE)) {  \
		printf("[LibBridge-TPM][ERROR] "format, ## __VA_ARGS__); } }while(0)

/// @brief 
/// @param nonce_str 
/// @param nonce_str_len 
/// @param nonce_timestamp 
/// @param out_smem_offset Required. 
/// @param out_tpm_phy_info Required. Updated with the TPM quote result content.
/// @return 
extern SAP_STATUS tpm_bridge_request_tpm_quote(
    char *nonce_str, 
    size_t nonce_str_len, 
    uint64_t nonce_timestamp, 
    smem_offset_t *out_smem_offset, 
    struct tpm_phy_info *out_tpm_phy_info
);

extern SAP_STATUS tpm_bridge_request_tpm_quote_free(smem_offset_t smem_offset);

extern SAP_STATUS tpm_bridge_request_secbase_seal_secrets_after_secos_termination(void);

#endif // __ASSEMBLY__
#endif // LIBBRIDGE_HEADERS_H