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

#ifndef TPM_BRIDGE_OPS_H
#define TPM_BRIDGE_OPS_H

#include <app_types.h>
#include <app_log.h>

#ifndef __ASSEMBLY__

struct tpm_bridge_msg
{
	uint32_t	cmd;
	uint8_t* 	req_data;
	uint8_t* 	resp_data;
};

#define TPM_BRIDGE_DEBUG(format, ...) \
	do{	\
		if((TPM_BRIDGE_LOG >= LOG_DEBUG) && (TPM_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[TPM-Bridge] "format, ## __VA_ARGS__); } }while(0)

#define TPM_BRIDGE_INFO(format, ...) \
	do{	\
        if((TPM_BRIDGE_LOG >= LOG_INFO) && (TPM_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[TPM-Bridge] "format, ## __VA_ARGS__); } }while(0)
#define TPM_BRIDGE_ERR(format, ...) \
	do{	\
        if((TPM_BRIDGE_LOG >= LOG_ERR) && (TPM_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[TPM-Bridge ERROR] "format, ## __VA_ARGS__); } }while(0)

extern bool tpm_bridge_init(void);

extern void tpm_bridge_fini(void);

extern bool tpm_bridge_send_red_os_release_input(void);

/// @brief 
/// @param msg 
/// @param out_valid_msg Required. Return true if the message contains a valid request to the TPM-bridge backend.
/// @return 
extern size_t handle_tpm_bridge_function(struct tpm_bridge_msg* msg, bool* out_valid_msg);

extern size_t do_CMD_TPM_NEW_EVENT(struct tpm_bridge_msg* msg);

#endif // __ASSEMBLY__
#endif // TPM_BRIDGE_OPS_H