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

#include <sap_headers.h>
#include <palddk.h>
#include <dtm_sp_intfs.h>

#include <stdio.h>

#include <gui/dtm/libdtm.h>
#include "comm_dtm.h"

extern void libdtm_hid_call_kbd_listeners(struct kbd_key_event* event);
extern void libdtm_hid_call_mouse_listeners(struct mouse_event* event);


/********* Private Functions *********/
static void do_SECSRV_DTM_SUBCMD_NOTIFY_EVENT(SP_MESSAGE* msg, COMM_ISP_SHAREDMEM* comm)
{
	DTM_EVENT event;
	
	// Copy the new DesktopMgr event, so it is less likely to be dropped if new event arrives at the DesktopMgr 
	memcpy(&event, (void*)comm->comm_buf, sizeof(DTM_EVENT));

	if(event.event_type == EVENT_TYPE_KBD)
		libdtm_hid_call_kbd_listeners(&event.kbd_event);
	else if (event.event_type == EVENT_TYPE_MOUSE)
		libdtm_hid_call_mouse_listeners(&event.mouse_event);

	return;
}

/********* Public Functions *********/
void libdtm_ops(SP_MESSAGE* msg, COMM_ISP_SHAREDMEM* comm)
{

#define HANDLE(hc) case hc: do_ ## hc (msg, comm); break
	switch(msg->header.sub_cmd)
	{
		HANDLE(SECSRV_DTM_SUBCMD_NOTIFY_EVENT);

		default:
			break; 
	}
#undef HANDLE

}