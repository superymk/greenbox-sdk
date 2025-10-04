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

#include <sys/ipc/varbuf.h>
#include <sys/ipc/comm.h>

#include <lib_bridge_hid_headers.h>

#include <app_sp_bridge_cmds.h>

// #include <app_sp_hid_bridge_defs.h>

/// @brief Timeout (in milli-seconds) to communication with HID-Bridge backend.
#define HID_BRIDGE_COMM_TIMEOUT_MS	(300)

static struct ipc_msg* _hid_bridge_create_msg(bool req_response, uint32_t cmd)
{
	struct ipc_msg* msg = NULL;

	// Build the request
	msg = sp_comm_app_create_msg(req_response);
	msg->header.cmd_type = HID_BRIDGE_OPS;
	msg->header.cmd = cmd;

	return msg;
}

SAP_STATUS hid_bridge_sent_kbd_event(struct kbd_key_event* event)
{
    struct hid_event hid_event = {0};
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	size_t msg_data_sz = 0;

	if(!event)
		return EINVAL;

	// Build the request
	req_msg = _hid_bridge_create_msg(true, CMD_HID_NEW_EVENT);
	if(!req_msg)
		goto err;

	// Build HID event to sent
	hid_event.hid_dev = HID_KEYBOARD;
	memcpy(&hid_event.kbd_event, event, sizeof(struct kbd_key_event));
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, &hid_event, sizeof(struct hid_event)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, HID_BRIDGE_COMM_TIMEOUT_MS);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);

	// On success
	return SUCCESS;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	return EINTERNAL;
}

SAP_STATUS hid_bridge_sent_mouse_event(struct mouse_event* event)
{
    struct hid_event hid_event = {0};
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	size_t msg_data_sz = 0;

	if(!event)
		return EINVAL;

	// Build the request
	req_msg = _hid_bridge_create_msg(true, CMD_HID_NEW_EVENT);

	// Build HID event to sent
	hid_event.hid_dev = HID_MOUSE;
	memcpy(&hid_event.mouse_event, event, sizeof(struct mouse_event));
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, &hid_event, sizeof(struct hid_event)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, HID_BRIDGE_COMM_TIMEOUT_MS);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);

	// On success
	return SUCCESS;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	return EINTERNAL;
}

/// @brief [Private Function][For DesktopMgr] Invoke DesktopMgr backend thread to set its priority to high.
/// @param  
/// @return 
SAP_STATUS hid_bridge_dtm_set_high_priority(void)
{
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;

	// Build the request
	req_msg = _hid_bridge_create_msg(true, CMD_HID_DTM_REQ_HIGH_PRIORITY);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, HID_BRIDGE_COMM_TIMEOUT_MS);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	if (!varbuf_read_fixdata(varbuf, &result, sizeof(result)))
    {
        BRIDGE_HID_ERR("Failed to get status!\n");
        goto err;
    }
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);

	if (result)
        return EINTERNAL;

	// On success
	return SUCCESS;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	return EINTERNAL;
}