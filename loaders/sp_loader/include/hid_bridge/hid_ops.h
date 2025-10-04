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

#ifndef HID_BRIDGE_OPS_H
#define HID_BRIDGE_OPS_H

#include <app_types.h>
#include <app_log.h>
#include "../ipc/app_comm_ops.h"

#ifndef __ASSEMBLY__

struct hid_bridge_msg
{
	uint32_t	cmd;
	uint8_t* 	req_data;
	uint8_t* 	resp_data;
};

#define HID_BRIDGE_DEBUG(format, ...) \
	do{	\
		if((HID_BRIDGE_LOG >= LOG_DEBUG) && (HID_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[HID-Bridge] "format, ## __VA_ARGS__); } }while(0)

#define HID_BRIDGE_INFO(format, ...) \
	do{	\
        if((HID_BRIDGE_LOG >= LOG_INFO) && (HID_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[HID-Bridge] "format, ## __VA_ARGS__); } }while(0)
#define HID_BRIDGE_ERR(format, ...) \
	do{	\
        if((HID_BRIDGE_LOG >= LOG_ERR) && (HID_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[HID-Bridge ERROR] "format, ## __VA_ARGS__); } }while(0)

bool hid_bridge_init(spid_t in_sp_id);

void hid_bridge_fini(void);

extern bool hid_bridge_send_red_os_release_input(COMM_SP_APP* comm);

/// @brief DesktopMgr backend thread uses this function to update its priority stored in SecAgent. So other backend 
/// threads can read the value from SecAgent and yield their CPU resources if needed.
/// @param need_high_priority 
/// @return 
extern int hid_bridge_dtm_thread_req_priority(bool need_high_priority);

/// @brief 
/// @param msg 
/// @param out_valid_msg Required. Return true if the message contains a valid request to the HID-bridge backend.
/// @return 
extern size_t handle_hid_bridge_function(struct hid_bridge_msg* msg, bool* out_valid_msg);

#endif // __ASSEMBLY__
#endif // HID_BRIDGE_OPS_H