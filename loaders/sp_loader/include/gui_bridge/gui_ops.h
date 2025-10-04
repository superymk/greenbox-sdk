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

#ifndef GUI_BRIDGE_OPS_H
#define GUI_BRIDGE_OPS_H

#include <app_types.h>
#include <app_log.h>

#ifndef __ASSEMBLY__

struct gui_bridge_msg
{
	uint32_t	cmd;
	uint8_t* 	req_data;
	uint8_t* 	resp_data;
};

#define GUI_BRIDGE_DEBUG(format, ...) \
	do{	\
		if((GUI_BRIDGE_LOG >= LOG_DEBUG) && (GUI_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[GUI-Bridge] "format, ## __VA_ARGS__); } }while(0)

#define GUI_BRIDGE_INFO(format, ...) \
	do{	\
        if((GUI_BRIDGE_LOG >= LOG_INFO) && (GUI_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[GUI-Bridge] "format, ## __VA_ARGS__); } }while(0)
#define GUI_BRIDGE_ERR(format, ...) \
	do{	\
        if((GUI_BRIDGE_LOG >= LOG_ERR) && (GUI_BRIDGE_LOG != LOG_NONE)) {  \
		sl_printf("[GUI-Bridge ERROR] "format, ## __VA_ARGS__); } }while(0)

extern bool gui_bridge_init(spid_t in_sp_id);
extern void gui_bridge_fini(void);

/// @brief Report the new window position
extern bool gui_bridge_send_new_window_pos(COMM_SP_APP* comm, uint32_t x, uint32_t y);

/// @brief Send the window restore and minimization event to the SecApp.
extern bool gui_bridge_send_window_restore_or_minimization(COMM_SP_APP* comm, bool window_restore);

#endif // __ASSEMBLY__
#endif // GUI_BRIDGE_OPS_H