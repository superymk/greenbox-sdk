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

#include <appddk.h>
#include <stdio.h>
#include <stdlib.h>

#include <ipc/varbuf.h>
#include <ipc/app_comm.h>
#include "secapp/app_sp_bridge_cmds.h"

#include "hid_bridge/hid_ops.h"

extern bool os_hid_bridge_init(void);
extern void os_hid_bridge_fini(void);

static spid_t sp_id = INVALID_SP_ID;

static struct ipc_msg *_hid_bridge_create_msg(COMM_SP_APP *comm, bool req_response, uint32_t cmd)
{
    struct ipc_msg *msg = NULL;

    msg = app_comm_sp_create_msg(comm, req_response);
    if (!msg)
        return NULL;

    msg->header.cmd_type = HID_BRIDGE_OPS;
    msg->header.cmd = cmd;

    return msg;
}

bool hid_bridge_send_red_os_release_input(COMM_SP_APP* comm)
{
    int ret = 0;
    OSSTATUS sa_status = OS_SUCCESS;
    struct ipc_msg *req_msg = NULL;

    // Check: params must be valid
    if(!comm)
        return false;

    // Build the request
    req_msg = _hid_bridge_create_msg(comm, false, CMD_HID_RED_OS_RELEASE_INPUT);

    // Send the request.
    ret = app_comm_sp_send_msg_nowait(comm, req_msg, 0);
    if (ret)
    {
        HID_BRIDGE_ERR("Failed to message SecProcess (%u)\n", sp_id);
        goto err;
    }

    // Signal the SecProcess
    sa_status = secagent_notify_secprocess(sp_id);
    if (SA_FAIL(sa_status))
    {
        HID_BRIDGE_ERR("Failed to notify SecProcess (%u)\n", sp_id);
        goto err;
    }

    // Do not wait on the response and just proceed. This is ok because missing some new window positions is acceptable.
    app_comm_sp_free_msg(req_msg, NULL);

    // On success
    return true;

err:
    app_comm_sp_free_msg(req_msg, NULL);
    return false;
}

bool hid_bridge_init(spid_t in_sp_id)
{
    bool status = true;

    sp_id = in_sp_id;

    status = os_hid_bridge_init();

    return status;
}

void hid_bridge_fini(void)
{
    os_hid_bridge_fini();

    sp_id = INVALID_SP_ID;
}

int hid_bridge_dtm_thread_req_priority(bool need_high_priority)
{
    OSSTATUS sa_status = OS_SUCCESS;
    uint32_t dtm_backend_thread_new_priority = 0;

    if(need_high_priority)
        dtm_backend_thread_new_priority = DTM_BACKEND_THREAD_PRIORITY_HIGHEST;
    else
        dtm_backend_thread_new_priority = DTM_BACKEND_THREAD_PRIORITY_NORMAL;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_set(NS_DTM_BACKEND_THREAD_FORCE_PRIORITY, (uint64_t)dtm_backend_thread_new_priority);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("Failed to share DesktopMgr backend thread priority with other NS processes! SecAgent status:%d\n", sa_status);
        return 1;
    }

    // On success
    return 0;
}