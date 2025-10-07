// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <libc_backend.h>
#include <hid_bridge/hid_ops.h>
#include <tpm_bridge/tpm_ops.h>
#include <stdlib.h>
#include "app_event_handler.h"

#include <ipc/app_comm_ops.h>
#include <app_sharedmem_mgr.h>

static bool app_prepare_response_msg(COMM_SP_APP *comm, struct ipc_msg *out_resp_msg, volatile struct ipc_msg *in_msg)
{
    bool status = true;

    status = app_comm_sp_prepare_request(comm, out_resp_msg, false);
    if (!status)
    {
        APPCOMM_ERR("Failed to generate message!\n");
        return false;
    }

    out_resp_msg->header.cmd_type = RESPONSE;
    out_resp_msg->header.ack_req_id = in_msg->header.req_id;
    out_resp_msg->header.req_response = 0;

    return true;
}

static void app_send_response_msg(COMM_SP_APP *comm, struct ipc_msg *out_resp_msg, size_t data_size)
{
    app_comm_sp_send_msg_nowait(comm, out_resp_msg, data_size);
}

/*static void dump_msg(uint8_t* data)
{
    int i = 0;
    for (i = 0; i < 64; i++)
    {
        uint32_t* p = (uint32_t*)data;

        printf("0x%08X ", p[i]);

        if( (i%8) == 7)
            printf("\n");
    }
    printf("\n");
}*/

bool ipc_handle_msg(COMM_SP_APP *comm, struct ipc_msg *req_msg, bool* out_valid_msg)
{
    struct ipc_msg resp_msg = {{0}};
    size_t msg_data_sz = 0;

    // Check: params must be valid
    if (!comm || !req_msg || !out_valid_msg)
        return false;

    APPCOMM_DEBUG("<ipc_handle_msg> CMD:%u. CMD Type:%u, ReqID:%u, Req response:%d\n",
                  req_msg->header.cmd, req_msg->header.cmd_type, req_msg->header.req_id, req_msg->header.req_response);

    *out_valid_msg = false;
    if (req_msg->header.cmd_type == LIBC_OPS)
    {
        struct libc_msg libc_msg;

        libc_msg.cmd = (uint32_t)req_msg->header.cmd;
        libc_msg.req_data = req_msg->req_data;
        libc_msg.resp_data = resp_msg.req_data;
        msg_data_sz = handle_libc_function(&libc_msg, out_valid_msg);

        // dump_msg(((struct ipc_msg*)msg)->req_data);
    }
    else if (req_msg->header.cmd_type == SHAREDMEM_OPS)
    {
        if (req_msg->header.cmd == SHAREMEM_ALLOCATE)
        {
            size_t size = 0;
            smem_offset_t offset = INVALID_SMEM_OFFSET;

            size = *(size_t *)req_msg->req_data;
            offset = smem_allocate(size);
            msg_data_sz = sizeof(offset);
            *(smem_offset_t *)(&resp_msg.req_data) = offset;

            *out_valid_msg = true;
        }
        else if (req_msg->header.cmd == SHAREMEM_FREE)
        {
            smem_offset_t offset = INVALID_SMEM_OFFSET;

            offset = *(smem_offset_t *)req_msg->req_data;
            smem_free(offset);

            *out_valid_msg = true;
        }
    }
    else if (req_msg->header.cmd_type == HID_BRIDGE_OPS)
    {
        struct hid_bridge_msg hid_bridge_msg;

        hid_bridge_msg.cmd = (uint32_t)req_msg->header.cmd;
        hid_bridge_msg.req_data = req_msg->req_data;
        hid_bridge_msg.resp_data = resp_msg.req_data;
        msg_data_sz = handle_hid_bridge_function(&hid_bridge_msg, out_valid_msg);
    }
    else if (req_msg->header.cmd_type == TPM_OPS)
    {
        struct tpm_bridge_msg tpm_bridge_msg;

        tpm_bridge_msg.cmd = (uint32_t)req_msg->header.cmd;
        tpm_bridge_msg.req_data = req_msg->req_data;
        tpm_bridge_msg.resp_data = resp_msg.req_data;
        msg_data_sz = handle_tpm_bridge_function(&tpm_bridge_msg, out_valid_msg);
    }

    // Response, if necessary
    if (req_msg->header.req_response)
    {
        bool status = true;

        // Init the content of the response msg
        status = app_prepare_response_msg(comm, &resp_msg, req_msg);
        if (!status)
        {
            APPCOMM_ERR("Handled the message:%u, but failed to generate message!\n", req_msg->header.req_id);
            return false;
        }

        // Send out the response msg
        app_send_response_msg(comm, &resp_msg, msg_data_sz);

        APPCOMM_DEBUG("<ipc_handle_msg> app_send_response_msg succeed\n");
    }

    return true;
}