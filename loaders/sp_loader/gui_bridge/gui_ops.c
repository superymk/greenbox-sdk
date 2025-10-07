// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h>
#include <stdlib.h>

#include <ipc/varbuf.h>
#include <ipc/app_comm.h>
#include "secapp/app_sp_bridge_cmds.h"

#include "gui_bridge/gui_ops.h"

static spid_t sp_id = INVALID_SP_ID;

static struct ipc_msg *_gui_bridge_create_msg(COMM_SP_APP *comm, bool req_response, uint32_t cmd)
{
    struct ipc_msg *msg = NULL;

    msg = app_comm_sp_create_msg(comm, req_response);
    if (!msg)
        return NULL;

    msg->header.cmd_type = GUI_BRIDGE_OPS;
    msg->header.cmd = cmd;

    return msg;
}

bool gui_bridge_send_new_window_pos(COMM_SP_APP* comm, uint32_t x, uint32_t y)
{
    int ret = 0;
    OSSTATUS sa_status = OS_SUCCESS;
    struct ipc_msg *req_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Check: params must be valid
    if(!comm)
        return false;

    // Build the request
    req_msg = _gui_bridge_create_msg(comm, false, CMD_GUI_NEW_WINDOW_POS);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &x, sizeof(uint32_t)))
        goto err;
    if (!varbuf_write_data(varbuf, &y, sizeof(uint32_t)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request.
    ret = app_comm_sp_send_msg_nowait(comm, req_msg, msg_data_sz);
    if (ret)
    {
        GUI_BRIDGE_ERR("Failed to message SecProcess (%u) about CMD_GUI_NEW_WINDOW_POS\n", sp_id);
        goto err;
    }

    // Signal the SecProcess
    sa_status = secagent_notify_secprocess(sp_id);
    if (SA_FAIL(sa_status))
    {
        GUI_BRIDGE_ERR("Failed to notify SecProcess (%u) about CMD_GUI_NEW_WINDOW_POS\n", sp_id);
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

bool gui_bridge_send_window_restore_or_minimization(COMM_SP_APP* comm, bool window_restore)
{
    int ret = 0;
    OSSTATUS sa_status = OS_SUCCESS;
    struct ipc_msg *req_msg = NULL;
    size_t msg_data_sz = 0;

    // Check: params must be valid
    if(!comm)
        return false;

    // Build the request
    if(window_restore)
        req_msg = _gui_bridge_create_msg(comm, false, CMD_GUI_WINDOW_RESTORE);
    else
        req_msg = _gui_bridge_create_msg(comm, false, CMD_GUI_WINDOW_HIDE);

    // Send the request.
    ret = app_comm_sp_send_msg_nowait(comm, req_msg, msg_data_sz);
    if (ret)
    {
        GUI_BRIDGE_ERR("Failed to message SecProcess (%u) about CMD_GUI_WINDOW_RESTORE/HIDE\n", sp_id);
        goto err;
    }

    // Signal the SecProcess
    sa_status = secagent_notify_secprocess(sp_id);
    if (SA_FAIL(sa_status))
    {
        if(window_restore)
            GUI_BRIDGE_ERR("Failed to notify SecProcess (%u) about CMD_GUI_WINDOW_RESTORE\n", sp_id);
        else
            GUI_BRIDGE_ERR("Failed to notify SecProcess (%u) about CMD_GUI_WINDOW_HIDE\n", sp_id);
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

bool gui_bridge_init(spid_t in_sp_id)
{
    sp_id = in_sp_id;

    return true;
}

void gui_bridge_fini(void)
{
    sp_id = INVALID_SP_ID;
}