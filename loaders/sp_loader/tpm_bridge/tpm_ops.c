// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#include <ipc/varbuf.h>
// #include "tpm_bridge/tpm_bridge_defs.h"
#include <secapp/app_sp_bridge_cmds.h>
#include "tpm_bridge/tpm_ops.h"

static bool _is_utpm_enabled = false;

static bool sb_is_utpm_enabled(void)
{
    int ret = 0;
    NS_HC_PARAMS_GET_SECBASE_BOOT_STATE sb_boot_state = {0};

    // Read the SecBase boot state that is measured in the TPM.
    ret = hypercall_secbase_get_boot_state(&sb_boot_state);
    if(ret)
    {
        TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read SecBase boot state error! ret:%d\n", ret);
        return false;
    }

    if(sb_boot_state.utpm.initialized)
        return true;
    else
        return false;

    // Should not hit
    return false;
}

static int _tpm_quote_serialize(
    NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT* sb_measurements, 
    char* tpm_quote_files_dirname, 
    char* evl_trace_file_dirname,
    struct tpm_phy_info* out_tpm_phy_info
)
{
    int ret = 0;
    size_t file_size = 0;

    // Check: SecBase must have already enable uTPM.
    // [NOTE] Enabling uTPM in SecBase is stronger than enabling physical TPM devices, but they are almost equal.
    if(!_is_utpm_enabled)
        return -1;

    // Read the machine ID from SecBase
    {
        ret = hypercall_secbase_get_unique_machine_id(out_tpm_phy_info->machine_id);
        if (ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read machine ID from SecBase error! ret:%d\n", ret);
            return -1;
        }
    }

    // Read the SecBase boot state that is measured in the TPM.
    {
        ret = hypercall_secbase_get_boot_state((NS_HC_PARAMS_GET_SECBASE_BOOT_STATE*)out_tpm_phy_info->secbase_boot_state);
        if(ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read SecBase boot state error! ret:%d\n", ret);
            return -1;
        }

        out_tpm_phy_info->secbase_boot_state_size = SECBASE_BOOT_STATE_SIZE;
    }

    // Read the quote result file and return the content.
    {
        char filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
        const char *aik_der_filename = "aik.der";
        const char *pcr1_filename = "pcr1.blob";
        const char *pcr2_filename = "pcr2.blob";
        const char *quote_blob_filename = "quote.blob";
        const char *quote_sig_filename = "quote.sig";

        //// Read "aik.der"
        snprintf(filepath, sizeof(filepath), "%s%s", tpm_quote_files_dirname, aik_der_filename);

        ret = file_read_check_size(filepath, 0, out_tpm_phy_info->aik_der, &file_size);
        if(ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read TPM AIK error!\n");
            return -1;
        }
        out_tpm_phy_info->aik_der_size = (uint32_t)file_size;

        //// Read "pcr1.blob"
        memset(filepath, 0, FS_MAX_PATH_BUFFER_LEN);
        snprintf(filepath, sizeof(filepath), "%s%s", tpm_quote_files_dirname, pcr1_filename);

        ret = file_read_check_size(filepath, SHA256_HASH_LENGTH, out_tpm_phy_info->secbase_image_pcr, NULL);
        if(ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read TPM PCR (SecBase image) error!\n");
            return -1;
        }

        //// Read "pcr2.blob"
        memset(filepath, 0, FS_MAX_PATH_BUFFER_LEN);
        snprintf(filepath, sizeof(filepath), "%s%s", tpm_quote_files_dirname, pcr2_filename);

        ret = file_read_check_size(filepath, SHA256_HASH_LENGTH, out_tpm_phy_info->secbase_boot_state_pcr, NULL);
        if(ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read TPM PCR (SecBase boot state) error!\n");
            return -1;
        }

        //// Read "quote.blob"
        memset(filepath, 0, FS_MAX_PATH_BUFFER_LEN);
        snprintf(filepath, sizeof(filepath), "%s%s", tpm_quote_files_dirname, quote_blob_filename);

        ret = file_read_check_size(filepath, TPM2_QUOTE_BLOB_SIZE, out_tpm_phy_info->quote_blob, &file_size);
        if (ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Failed to read quote.blob!\n");
            return -1;
        }
        out_tpm_phy_info->quote_blob_size = (uint32_t)file_size;

        //// Read "quote.sig"
        memset(filepath, 0, FS_MAX_PATH_BUFFER_LEN);
        snprintf(filepath, sizeof(filepath), "%s%s", tpm_quote_files_dirname, quote_sig_filename);

        ret = file_read_check_size(filepath, TPM2_QUOTE_SIGNATURE_LENGTH, out_tpm_phy_info->quote_sig, NULL);
        if (ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Failed to read quote.sig!\n");
            return -1;
        }
    }

    // Read the history of physical TPM's PCRs that measure SecBase's boot states.
    {
        if(sb_measurements->boot_state_pcr_orig_size != SHA256_HASH_LENGTH)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Unexpected length of TPM PCR (SecBase boot state)! \n");
            return -1;
        }

        memcpy(out_tpm_phy_info->boot_state_pcr_orig, sb_measurements->boot_state_pcr_orig, SHA256_HASH_LENGTH);
    }

    // Read the history of physical TPM's PCRs that measure SecBase's image (i.e., EFI TPM event log trace) from both 
    // the untrusted OS and SecBase. Then use the one with more entries.
    // [NOTE] Some OSes (e.g., older Linux, customized Linux) may not export the event logs at all. Thus, SecBase has to
    // provide a hypercall to export the event logs as a fall back solution for these OSes.
    {
        //// Try to read the trace result from untrusted OS
        char filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
        const char *trace_filename = "evl_trace.blob";
        SECBASE_TPM_EVENT_LOG_TRACE os_img_state_trace = {0};
        SECBASE_TPM_EVENT_LOG_TRACE* use_trace = NULL;

        snprintf(filepath, sizeof(filepath), "%s%s", evl_trace_file_dirname, trace_filename);

        ret = file_read_check_size(filepath, sizeof(SECBASE_TPM_EVENT_LOG_TRACE), &os_img_state_trace, NULL);
        if(ret)
        {
            TPM_BRIDGE_ERR("<_tpm_quote_serialize> Read OS TPM event log trace error!\n");
            return -1;
        }

        out_tpm_phy_info->img_state_trace_source = IMG_STATE_PCR_TRACE_FROM_INVALID;
        if(os_img_state_trace.count >= sb_measurements->img_state_trace.count)
        {
            // By default, we prefer to use the trace reported by the untrusted OS, because it contains PCR measurements
            // after SecBase boots.
            use_trace = &os_img_state_trace;
            out_tpm_phy_info->img_state_trace_source = IMG_STATE_PCR_TRACE_FROM_OS;
        }
        else
        {
            // Otherwise, use the trace returned from SecBase.
            use_trace = &sb_measurements->img_state_trace;
            out_tpm_phy_info->img_state_trace_source = IMG_STATE_PCR_TRACE_FROM_SECBASE;
        }

        memcpy(&out_tpm_phy_info->img_state_trace, use_trace, sizeof(SECBASE_TPM_EVENT_LOG_TRACE));
    }

    // On success
    return 0;
}

static size_t do_CMD_TPM_QUOTE(struct tpm_bridge_msg* msg)
{
    int ret = 0;
	char* nonce_str = NULL;
    uint64_t nonce_timestamp = 0;
    char tpm_quote_files_dirname[FS_MAX_PATH_BUFFER_LEN] = {0};
    char evl_trace_file_dirname[FS_MAX_PATH_BUFFER_LEN] = {0};
    SL_STATUS sl_status = SL_SUCCESS;
    NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT sb_measurements = {0};
    
    smem_offset_t smem_offset = INVALID_SMEM_OFFSET;

    uint64_t result = -1;
	varbuf_t* varbuf = NULL;
    size_t msg_data_sz = 0;

	varbuf = varbuf_access_start(msg->req_data);
	nonce_str = (char*)varbuf_read_vardata(varbuf, NULL);
    if(!nonce_str)
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Get nonce string error!\n");
		goto out;
    }
    if(!varbuf_read_fixdata(varbuf, &nonce_timestamp, sizeof(uint64_t)))
	{
		TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Get nonce timestamp error!\n");
		goto out;
	}
	varbuf_access_finish(varbuf);

    // Check: SecBase must have already enable uTPM.
    // [NOTE] Enabling uTPM in SecBase is stronger than enabling physical TPM devices, but they are almost equal.
    if(!_is_utpm_enabled)
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE: SecBase does not enable uTPM/TPM!\n");
		goto out;
    }

    // Get SecBase's TPM measurements
    ret = hypercall_secbase_get_tpm_pcrs(&sb_measurements);
    if(ret)
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Get SecBase's TPM measurements error! ret:%d\n", ret);
		goto out;
    }

	// Perform TPM operations
    ret = app_sl_rpc_call_sl_secbase_tpm_get_quote(nonce_str, nonce_timestamp, sb_measurements.img_pcr_idx, sb_measurements.boot_state_pcr_idx, &sl_status, tpm_quote_files_dirname);
    if(ret || SL_FAIL(sl_status))
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Failed to generate TPM quote! ret:%d, sl_status:%lu\n", ret, sl_status);
		goto out;
    }

    ret = app_sl_rpc_call_sl_secbase_tpm_get_pcrs(sb_measurements.img_pcr_idx, sb_measurements.boot_state_pcr_idx, &sl_status, NULL);
    if(ret || SL_FAIL(sl_status))
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Failed to get TPM PCRs! ret:%d, sl_status:%lu\n", ret, sl_status);
		goto out;
    }

    ret = app_sl_rpc_call_sl_os_efi_get_tpm2_event_logs_trace(sb_measurements.img_pcr_idx, &sl_status, evl_trace_file_dirname);
    if(ret || SL_FAIL(sl_status))
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Failed to get EFI TPM event log trace! ret:%d, sl_status:%lu\n", ret, sl_status);
		goto out;
    }

    // Serialize the TPM quote
    {
        struct tpm_phy_info* info = NULL;

        smem_offset = smem_allocate(sizeof(struct tpm_phy_info));
        if(smem_offset == INVALID_SMEM_OFFSET)
        {
            TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Failed to allocate TPM quote result in shared memory!\n");
		    goto out;
        }

        info = (struct tpm_phy_info*)SMEM_OFFSET_TO_VADDR(smem_offset);
        ret = _tpm_quote_serialize(&sb_measurements, tpm_quote_files_dirname, evl_trace_file_dirname, info);
        if(ret)
        {
            TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Failed to serialize TPM quote result!\n");
            smem_free(smem_offset);
		    goto out;
        }
    }
    
    // On success
	result = 0;

out:
    // return the result
	varbuf = varbuf_access_start(msg->resp_data);
    if(!varbuf_write_data(varbuf, &result, sizeof(result)))
		TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Return status Failed\n");
    if(!varbuf_write_data(varbuf, &smem_offset, sizeof(smem_offset)))
		TPM_BRIDGE_ERR("CMD_TPM_QUOTE: Return <smem_offset> Failed\n");
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	return msg_data_sz;
}

static size_t do_CMD_TPM_QUOTE_FREE(struct tpm_bridge_msg* msg)
{
    smem_offset_t smem_offset = INVALID_SMEM_OFFSET;

    uint64_t result = -1;
	varbuf_t* varbuf = NULL;
    size_t msg_data_sz = 0;

	varbuf = varbuf_access_start(msg->req_data);
    if(!varbuf_read_fixdata(varbuf, &smem_offset, sizeof(smem_offset_t)))
	{
		TPM_BRIDGE_ERR("CMD_TPM_QUOTE_FREE: Request Failed: <smem_offset>\n");
		goto out;
	}

    // Check: SecBase must have already enable uTPM.
    // [NOTE] Enabling uTPM in SecBase is stronger than enabling physical TPM devices, but they are almost equal.
    if(!_is_utpm_enabled)
    {
        TPM_BRIDGE_ERR("CMD_TPM_QUOTE_FREE: SecBase does not enable uTPM/TPM!\n");
		goto out;
    }

    if(smem_offset != INVALID_SMEM_OFFSET)
    {
        smem_free(smem_offset);
    }

    // On success
	result = 0;

out:
    // return the result
	varbuf = varbuf_access_start(msg->resp_data);
    if(!varbuf_write_data(varbuf, &result, sizeof(result)))
		TPM_BRIDGE_ERR("CMD_TPM_QUOTE_FREE: Return status Failed\n");
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	return msg_data_sz;
}

/// @brief SecProcess requests SOSLoader to seal secbase secrets right after SecOS unload.
/// @param msg 
/// @return 
static size_t do_CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD(struct tpm_bridge_msg* msg)
{
    int ret = 0;
    SL_STATUS sl_status = SL_SUCCESS;

    uint64_t result = -1;
	varbuf_t* varbuf = NULL;
    size_t msg_data_sz = 0;

    // Check: SecBase must have already enable uTPM.
    // [NOTE] Enabling uTPM in SecBase is stronger than enabling physical TPM devices, but they are almost equal.
    if(!_is_utpm_enabled)
    {
        TPM_BRIDGE_ERR("CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD: SecBase does not enable uTPM/TPM!\n");
		goto out;
    }

    // Call SOSLoader to run the task after SecOS unload.
    ret = app_sl_rpc_call_sl_mark_tasks_after_secos_unload(TASK_AFTER_SECOS_UNLOAD_SECBASE_SECRETS_SEAL, &sl_status);
    if(ret || SL_FAIL(sl_status))
    {
        TPM_BRIDGE_ERR("CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD: Failed to add the task after SecOS unload! ret:%d, sl_status:%lu\n", ret, sl_status);
		goto out;
    }

    // On success
	result = 0;

out:
    // return the result
	varbuf = varbuf_access_start(msg->resp_data);
    if(!varbuf_write_data(varbuf, &result, sizeof(result)))
		TPM_BRIDGE_ERR("CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD: Return status Failed\n");
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	return msg_data_sz;
}


#define HANDLE(hc) case hc: msg_data_sz = do_ ## hc (msg); break

size_t handle_tpm_bridge_function(struct tpm_bridge_msg* msg, bool* out_valid_msg)
{
	size_t msg_data_sz = 0;
    bool valid_msg = true;
	
	switch(msg->cmd)
	{
		// Keyboard/mouse events
		HANDLE(CMD_TPM_QUOTE);
        HANDLE(CMD_TPM_QUOTE_FREE);
        HANDLE(CMD_TPM_REQ_SECBASE_SEAL_SECRETS_AFTER_SECOS_UNLOAD);
		
		default:
			APP_ERR("Invalid Command!\n");
            valid_msg = false;
			break;
	}

    *out_valid_msg = valid_msg;

	return msg_data_sz;
}




/********* Public Functions *********/
bool tpm_bridge_init(void)
{
    int ret = 0;

    // Connect to SOSLoader to use its TPM functions on-demand.
    ret = app_sl_rpc_init();
    if(ret)
    {
        TPM_BRIDGE_ERR("Failed to initialize\n");
        return false;
    }

    // Query SecBase about whether the uTPM is enabled
    _is_utpm_enabled = sb_is_utpm_enabled();

    // On success
    TPM_BRIDGE_INFO("Initialized\n");
    return true;
}

void tpm_bridge_fini(void)
{
    app_sl_rpc_fini();
}