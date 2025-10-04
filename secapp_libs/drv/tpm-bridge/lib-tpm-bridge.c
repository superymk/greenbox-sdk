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

#include <lib_bridge_tpm_headers.h>

// #include <app_sp_tpm_bridge_defs.h>

static struct ipc_msg *_tpm_bridge_create_msg(bool req_response, uint32_t cmd)
{
    struct ipc_msg *msg = NULL;

    // Build the request
    msg = sp_comm_app_create_msg(req_response);
    msg->header.cmd_type = TPM_OPS;
    msg->header.cmd = cmd;

    return msg;
}

SAP_STATUS tpm_bridge_request_tpm_quote(char *nonce_str, size_t nonce_str_len, uint64_t nonce_timestamp, smem_offset_t *out_smem_offset, struct tpm_phy_info *out_tpm_phy_info)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;
    uint64_t result = -1;
    smem_offset_t smem_offset = INVALID_SMEM_OFFSET;
    size_t nonce_str_mem_len = nonce_str_len + 1;

    // Check: params must be valid
    if (!nonce_str || !out_smem_offset || !out_tpm_phy_info)
        return EINVAL;
    if (nonce_str_mem_len >= TPM_QUOTE_NONCE_STRING_MAX_LENGTH)
        return EINVAL;

    // Build the request
    req_msg = _tpm_bridge_create_msg(true, CMD_TPM_QUOTE);
    if (!req_msg)
        goto err;

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_str(varbuf, nonce_str, nonce_str_mem_len))
        goto err;
    if (!varbuf_write_data(varbuf, &nonce_timestamp, sizeof(uint64_t)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    // [NOTE] Get TPM quote may takes several seconds. So we wait for 10 seconds.
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 10000);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_read_fixdata(varbuf, &result, sizeof(result)))
    {
        BRIDGE_TPM_ERR("Failed to get status!\n");
        goto err;
    }
    if (!varbuf_read_fixdata(varbuf, &smem_offset, sizeof(smem_offset_t)))
    {
        BRIDGE_TPM_ERR("Failed to get shared memory offset!\n");
        goto err;
    }
    varbuf_access_finish(varbuf);

    // Read out the result TPM quote
    {
        struct tpm_phy_info *info = NULL;

        info = (struct tpm_phy_info *)SMEM_OFFSET_TO_VADDR(smem_offset);
        if (!info)
        {
            BRIDGE_TPM_ERR("Failed to get TPM quote from shared memory!\n");
            goto err;
        }

        memcpy(out_tpm_phy_info, info, sizeof(struct tpm_phy_info));
        *out_smem_offset = smem_offset;
    }

    sp_comm_app_free_msg(req_msg, resp_msg);

    if (result)
        return EINTERNAL;

    // On success
    return SUCCESS;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return EINTERNAL;
}

SAP_STATUS tpm_bridge_request_tpm_quote_free(smem_offset_t smem_offset)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;
    uint64_t result = -1;

    // Build the request
    req_msg = _tpm_bridge_create_msg(true, CMD_TPM_QUOTE_FREE);
    if (!req_msg)
        goto err;

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &smem_offset, sizeof(smem_offset)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 1000);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_read_fixdata(varbuf, &result, sizeof(result)))
    {
        BRIDGE_TPM_ERR("Failed to get status!\n");
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

SAP_STATUS tpm_bridge_request_secbase_seal_secrets_after_secos_termination(void)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;
    uint64_t result = -1;

    // Build the request
    req_msg = _tpm_bridge_create_msg(true, CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD);
    if (!req_msg)
        goto err;

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 1000);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_read_fixdata(varbuf, &result, sizeof(result)))
    {
        BRIDGE_TPM_ERR("Failed to get status!\n");
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