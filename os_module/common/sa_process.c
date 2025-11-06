// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

// The SOS-OS communication channel timeout for SecProcess operations is 3000ms.
#define SP_COMM_SOS_TIMEOUT (3000)

//! Request SecOS to allocate shared memory 
static OSSTATUS _sp_get_id_name_batch(offset_t* out_offset)
{
    OSSTATUS status = OS_SUCCESS;
	struct comm_os_msg request = {{0}};
	struct comm_os_msg response = {{0}};
	
	// Fill the msg header 
	request.header.data_size = 0;
	request.header.cmd = CMD_SECOS_SP_INFO_GET_IDS_NAMES;
	status = comm_sos_prepare_request(&request, true, SECOS_ID);
	if(SA_FAIL(status))
    {
        DRV_ERR("SecProcess batch get IDs and names error! Failed to generate request to SecOS! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

	// Send it out
	status = comm_sos_send_msg_and_wait(&request, &response, SP_COMM_SOS_TIMEOUT);
	if(SA_FAIL(status))
	{
		DRV_ERR("SecProcess batch get IDs and names error! Send msg to SecOS error! status:%lu\n", status);
		return E_COMM_ERR;
	}

	// Response
	if(response.header.response != SOS_COMM_SUCCESS)
	{
		DRV_ERR("SecProcess batch get IDs and names error! SecOS returns memory error.\n");
		return E_NO_MEM;
	}
	
	*out_offset = *(offset_t*)response.req_data;
	return OS_SUCCESS;
}

static OSSTATUS _sp_get_id_name_batch_free_sharedmem(offset_t offset)
{
    return sos_sharedmem_free(offset, sizeof(struct sp_info_id_name_resp));
}




/********* Public Functions *********/
OSSTATUS do_CMD_REGISTER_SECPROCESS_MEMORY(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    struct hv_reg_sections* in_scode_info_ptr = NULL;
    struct hv_reg_sections scode_info = {0};
    ulong_t ret = 0;
    uint32_t i = 0;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet || !cmd_packet->arg1)
        return E_INV_ARG;
    in_scode_info_ptr = (struct hv_reg_sections*)(vaddr_t)cmd_packet->arg1;

    // Copy parameters
    status = os->copy_from_user(&scode_info, in_scode_info_ptr, sizeof(struct hv_reg_sections));
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS_MEMORY] Copy user params error!\n");
        status = E_INTERNAL_ERROR;
        goto register_sp_mem_copy_params_err;
    }

    // Check: The current process must have RW permission to all SecProcess sections
    FOREACH_S(i, scode_info.num_sections, TV_MAX_SECTIONS, 0, 1)
    {
        bool allowed_read_write = false;
        struct section_meminfo* section = &scode_info.sections[i];

        status = os->mm_check_current_process_page_permission(section->start_addr, section->page_num * PAGE_SIZE, &allowed_read_write);
        if(SA_FAIL(status))
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS_MEMORY] Failed to check SecProcess loader's memory permissions of section %u, start vaddr:0x%lX, pages:%lu!\n", 
                i, section->start_addr, section->page_num);
            status = E_INTERNAL_ERROR;
            goto register_sp_mem_check_perm_err;
        }
        if(!allowed_read_write)
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS_MEMORY] SecProcess loader does not have RW permission to section %u, start vaddr:0x%lX, pages:%lu!\n",
                i, section->start_addr, section->page_num);
            status = E_CHKERR;
            goto register_sp_mem_check_perm_err;
        }
    }
    
    // Call SecBase
    ret = os->hypercall_1arg(NS_HC_ASSIGN_MMEM_TO_S, (vaddr_t)&scode_info);
    if (ret)
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS_MEMORY] Register SecProcess via hypercall error! ret:%lu\n", ret);
        status = E_INTERNAL_ERROR;
        goto register_sp_mem_call_secbase_err;
    }
    
    DRV_DEBUG("[IOCTL-CMD_REGISTER_SECPROCESS_MEMORY] Success\n");
    status = OS_SUCCESS;
    goto register_sp_mem_ret;
    

register_sp_mem_ret:
register_sp_mem_call_secbase_err:
register_sp_mem_check_perm_err:
register_sp_mem_copy_params_err:
    return status;
}

OSSTATUS do_CMD_REGISTER_SECPROCESS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    OSSTATUS fini_status = OS_SUCCESS;
    offset_t sp_params_offset_in_sharedmem = 0;
    vaddr_t sp_params_sharedmem_vaddr = INVALID_VADDR;
    SP_USTRUCT* usr_params_ptr = NULL;
    SP_USTRUCT* params = NULL;
    struct comm_os_msg* request = NULL;
    struct comm_os_msg* response = NULL;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet || !cmd_packet->arg1)
        return E_INV_ARG;
    usr_params_ptr = (SP_USTRUCT*)(vaddr_t)cmd_packet->arg1;

    params = os->mm_malloc_zeroed(sizeof(SP_USTRUCT));
    if(!params)
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Allocate variables error!\n");
        status = E_INTERNAL_ERROR;
        goto register_sp_ret;
    }

    // Copy user params
    // [NOTE] We must copy the params before checking, or suffers TOCTTOU attacks
    // [TODO][Issue 22] In <CMD_REGISTER_SECPROCESS>, the module needs not to copy <sp_additional_params> on app's request
    status = os->copy_from_user(params, usr_params_ptr, sizeof(SP_USTRUCT));
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Copy parameters error!\n");
        status = E_INV_ARG;
        goto register_sp_ret;
    }

    // Check user params
    //// Check <params->app_pid>
    if(params->app_pid != os->get_current_pid())
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Invalid parameter <app_pid>!\n");
        status = E_INV_ARG;
        goto register_sp_ret;
    }
    //// No need to check other parameters, SecOS can check them.

    // Send user params to SecOS
    status = sos_sharedmem_alloc(&sp_params_offset_in_sharedmem, 0, sizeof(SP_USTRUCT));
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Failed to request SecOS to allocate shared memory!\n");
        status = E_NO_MEM;
        goto register_sp_ret;
    }

    //// Copy params
    sp_params_sharedmem_vaddr = sos_sharedmem_ptr(sp_params_offset_in_sharedmem);
    if(sp_params_sharedmem_vaddr == INVALID_VADDR)
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Cannot get the memory pointer of SecOS's shared memory!\n");
        status = E_NO_MEM;
        goto register_sp_copy_params_err;
    }
    os->mm_memcpy((void*)sp_params_sharedmem_vaddr, params, sizeof(SP_USTRUCT));
    
    //// Send msg
    {
        request = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));
        response = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));

        if(!request || !response)
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Failed to allocate msg buffers to communicate with SecOS!\n");
            status = E_NO_MEM;
            goto register_sp_mem_err;
        }
        
        // Fill the msg header 
        request->header.data_size = sizeof(sp_params_offset_in_sharedmem);
        request->header.cmd = CMD_SECOS_SECPROCESS_CREATE;
        *(offset_t*)request->req_data = sp_params_offset_in_sharedmem;
        status = comm_sos_prepare_request(request, true, SECOS_ID);
        if(SA_FAIL(status))
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Failed to generate request to SecOS! status:%lu\n", status);
            status = E_INTERNAL_ERROR;
            goto register_sp_build_msg_err;
        }

        // Send it out
        status = comm_sos_send_msg_and_wait(request, response, SP_COMM_SOS_TIMEOUT);
        if(SA_FAIL(status))
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Failed to submit SecProcess's parameters to SecOS! status:%lu\n", status);
            status = E_TIMEOUT;
            goto register_sp_send_msg_err;
        }

        // Response
        if(response->header.response != SOS_COMM_SUCCESS)
        {
            DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] SecOS cannot create the SecProcess!\n");
            status = E_SECOS_ERROR;
            goto register_sp_send_msg_err;
        }
    }

    // Free shared mem that carries user params
    status = sos_sharedmem_free(sp_params_offset_in_sharedmem, sizeof(SP_USTRUCT));
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Failed to request SecOS to free shared memory!\n");
        status = E_INTERNAL_ERROR;
        goto register_sp_free_sharedmem_err;
    }

    // On success, return
    DRV_DEBUG("[IOCTL-CMD_REGISTER_SECPROCESS] Success\n");
    status = OS_SUCCESS;
    goto register_sp_ret;


register_sp_send_msg_err:
register_sp_build_msg_err:
register_sp_mem_err:
register_sp_copy_params_err:
    fini_status = sos_sharedmem_free(sp_params_offset_in_sharedmem, sizeof(SP_USTRUCT));
    if(SA_FAIL(fini_status))
    {
        DRV_ERR("[IOCTL-CMD_REGISTER_SECPROCESS] Failed to request SecOS to free shared memory! status:%lu\n", fini_status);
    }
register_sp_free_sharedmem_err:
register_sp_ret:
    if(request)
        os->mm_free(request);
    if(response)
        os->mm_free(response);
    if(params)
        os->mm_free(params);
    
    return status;
}

OSSTATUS do_CMD_RUN_SECPROCESS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    struct comm_os_msg* msg = NULL;
    sp_id_t sp_id = INVALID_SP_ID;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;
    sp_id = (sp_id_t)cmd_packet->arg1;

    DRV_INFO("Starting SecProcess (%u)\n",  (uint32_t)sp_id);
    msg = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));

    if(!msg)
    {
        DRV_ERR("[IOCTL-CMD_RUN_SECPROCESS] Failed to allocate msg buffers to communicate with SecOS!\n");
        status = E_NO_MEM;
        goto run_sp_mem_err;
    }
    
    // Fill the msg header 
    msg->header.data_size = sizeof(sp_id);
    msg->header.cmd = CMD_SECOS_SECPROCESS_RUN;
    *(uint32_t*)msg->req_data = sp_id;
    status = comm_sos_prepare_request(msg, false, SECOS_ID);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_RUN_SECPROCESS] Failed to generate request to SecOS! status:%lu\n", status);
        status = E_INTERNAL_ERROR;
        goto run_sp_comm_sos_build_msg_err;
    }

    // Send it out
    status = comm_sos_send_msg_nowait(msg);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_RUN_SECPROCESS] Failed to submit commands to SecOS!\n");
        status = E_COMM_ERR;
        goto run_sp_comm_sos_send_err;
    }

    // No response is needed
    DRV_DEBUG("[IOCTL-CMD_RUN_SECPROCESS] Success\n");
    status = OS_SUCCESS;
    goto run_sp_ret;


run_sp_ret:
run_sp_comm_sos_send_err:
run_sp_comm_sos_build_msg_err:
run_sp_mem_err:
    if(msg)
        os->mm_free(msg);
    
    return status;
}

OSSTATUS do_CMD_SOFT_TERMINATE_SECPROCESS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    struct comm_os_msg* request = NULL;
    sp_id_t sp_id = INVALID_SP_ID;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;
    sp_id = (sp_id_t)cmd_packet->arg1;

    DRV_INFO("Soft-Terminating SecProcess (%u)\n", (uint32_t)sp_id);
    request = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));

    if(!request)
    {
        DRV_ERR("[IOCTL-CMD_SOFT_TERMINATE_SECPROCESS] Failed to allocate msg buffers to communicate with SecOS!\n");
        status = E_NO_MEM;
        goto soft_terminate_sp_mem_err;
    }
    
    // Fill the msg header 
    request->header.data_size = 0;
    request->header.cmd = CMD_SECPROCESS_SOFT_EXIT;
    status = comm_sos_prepare_request(request, false, sp_id);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_SOFT_TERMINATE_SECPROCESS] Failed to generate request to SecOS! status:%lu\n", status);
        status = E_INTERNAL_ERROR;
        goto soft_terminate_sp_build_msg_err;
    }

    // Send it out
    status = comm_sos_send_msg_nowait(request);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_SOFT_TERMINATE_SECPROCESS] Failed to submit commands to SecProcess (%u)!\n", (uint32_t)sp_id);
        status = E_COMM_ERR;
        goto soft_terminate_sp_send_msg_err;
    }

    // No response is needed
    DRV_DEBUG("[IOCTL-CMD_SOFT_TERMINATE_SECPROCESS] Success\n");
    status = OS_SUCCESS;
    goto soft_terminate_sp_ret;


soft_terminate_sp_ret:
soft_terminate_sp_send_msg_err:
soft_terminate_sp_build_msg_err:	
soft_terminate_sp_mem_err:
    if(request)
        os->mm_free(request);
    
    return status;
}

OSSTATUS do_CMD_FORCE_TERMINATE_SECPROCESS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    struct comm_os_msg* request = NULL;
    sp_id_t sp_id = INVALID_SP_ID;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;
    sp_id = (sp_id_t)cmd_packet->arg1;

    DRV_INFO("Force-Terminating SecProcess (%u)\n", (uint32_t)sp_id);
    request = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));

    if(!request)
    {
        DRV_ERR("[IOCTL-CMD_FORCE_TERMINATE_SECPROCESS] Failed to allocate msg buffers to communicate with SecOS!\n");
        status = E_NO_MEM;
        goto terminate_sp_mem_err;
    }
    
    // Fill the msg header 
    request->header.data_size = sizeof(sp_id);
    request->header.cmd = CMD_SECOS_SECPROCESS_FORCE_EXIT;
    *(sp_id_t*)request->req_data = sp_id;
    status = comm_sos_prepare_request(request, false, SECOS_ID);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_FORCE_TERMINATE_SECPROCESS] Failed to generate request to SecOS! status:%lu\n", status);
        status = E_INTERNAL_ERROR;
        goto terminate_sp_build_msg_err;
    }

    // Send it out
    status = comm_sos_send_msg_nowait(request);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_FORCE_TERMINATE_SECPROCESS] Failed to submit commands to SecOS!\n");
        status = E_COMM_ERR;
        goto terminate_sp_send_msg_err;
    }

    // No response is needed
    DRV_DEBUG("[IOCTL-CMD_FORCE_TERMINATE_SECPROCESS] Success\n");
    status = OS_SUCCESS;
    goto terminate_sp_ret;


terminate_sp_ret:
terminate_sp_send_msg_err:
terminate_sp_build_msg_err:
terminate_sp_mem_err:
    if(request)
        os->mm_free(request);
    
    return status;
}

OSSTATUS do_CMD_NOTIFY_SECPROCESS(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    uint32_t id_sap = 0;
    // vaddr_t app_req_msg = INVALID_VADDR;
    // size_t req_msg_size = 0;
    struct comm_os_msg* msg = NULL;
    // int ret = 0;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;
    id_sap = (uint32_t)cmd_packet->arg1;
    // app_req_msg = (vaddr_t)cmd_packet->arg2;
    // req_msg_size = (size_t)cmd_packet->arg3;

    msg = (struct comm_os_msg*)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));

    // Fill the msg header 
    // msg->header.data_size = req_msg_size;
    msg->header.data_size = 0;

    msg->header.cmd = CMD_SECPROCESS_NOTIFY;
    status = comm_sos_prepare_request(msg, false, id_sap);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_NOTIFY_SECPROCESS] Failed to generate request to SecOS! status:%lu\n", status);
        status = E_INTERNAL_ERROR;
        goto req_sp_build_msg_err;
    }

    // [NOTE] Currently we did not see any necessity to send data along with a notification to a SecProcess. So we do not
    // copy any data here. A red application can always do so by sending data in App-SecProcess communication channels.
    
    // status = os->copy_from_user(msg->req_data, (void*)app_req_msg, req_msg_size);
    // if(SA_FAIL(status))
    // {
    //     DRV_ERR("[IOCTL-CMD_NOTIFY_SECPROCESS] Copy from user error!\n");
    //     status = E_INTERNAL_ERROR;
    //     goto req_sp_copy_params_err;
    // }
    
    // Send it out
    status = comm_sos_send_msg_nowait(msg);
    if(SA_FAIL(status))
    {
        DRV_ERR("[IOCTL-CMD_NOTIFY_SECPROCESS] Failed to submit SecProcess's parameters to SecOS!\n");
        status = E_COMM_ERR;
        goto req_sp_send_msg_err;
    }

    // On success, return
    DRV_DEBUG("[IOCTL-CMD_REGISTER_SECPROCESS] Success\n");
    status = OS_SUCCESS;
    goto req_sp_ret;


req_sp_ret:
req_sp_send_msg_err:
// req_sp_copy_params_err:
req_sp_build_msg_err:
    if(msg)
        os->mm_free(msg);

    return status;
}

OSSTATUS do_CMD_SECPROCESS_GET_IDS_AND_NAMES(struct tz_cmd_packet *cmd_packet)
{
    OSSTATUS status = OS_SUCCESS;
    offset_t resp_offset_in_sharedmem = 0;
    vaddr_t resp_vaddr_sharedmem = INVALID_VADDR;
    vaddr_t usr_resp_buf_uvaddr = INVALID_VADDR;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet || !cmd_packet->arg1)
        return E_INV_ARG;
    usr_resp_buf_uvaddr = (vaddr_t)cmd_packet->arg1;

    // Check: SecOS must be running
    if(!sos_is_running)
        return E_INV_ARG;

    // Get the SecProcesses' names and IDs
    status = _sp_get_id_name_batch(&resp_offset_in_sharedmem);
    if(SA_FAIL(status))
	{
		DRV_ERR("[IOCTL-CMD_SECPROCESS_GET_IDS_AND_NAMES] Get SecProcesses' names and IDs error! status:%lu\n", status);
		return E_COMM_ERR;
	}
    resp_vaddr_sharedmem = sos_sharedmem_ptr(resp_offset_in_sharedmem);
    if(resp_vaddr_sharedmem == INVALID_VADDR)
    {
        DRV_ERR("[IOCTL-CMD_SECPROCESS_GET_IDS_AND_NAMES] Cannot get the memory pointer of SecOS's response in SecOS-OS shared memory!\n");
        status = E_NO_MEM;
        goto get_resp_vaddr_error;
    }

    // Copy the result to <usr_resp_buf>
    status = os->copy_to_user((void*) usr_resp_buf_uvaddr, (void*)resp_vaddr_sharedmem, sizeof(struct sp_info_id_name_resp)); 
    if (SA_FAIL(status))
    {
        SACOMM_ERR("[IOCTL-CMD_SECPROCESS_GET_IDS_AND_NAMES] Copy to user error! status:%lu\n", status);
        status = E_INTERNAL_ERROR;
        goto copy_to_usr_err;
    }

    // Free the info in shared mem
    status = _sp_get_id_name_batch_free_sharedmem(resp_offset_in_sharedmem);
    if(SA_FAIL(status))
	{
		DRV_ERR("[IOCTL-CMD_SECPROCESS_GET_IDS_AND_NAMES] Failed to free the response in SecOS-OS shared memory! status:%lu\n", status);
		status = E_INTERNAL_ERROR;
        goto free_sharedmem_err;
	}

    // On success
    return OS_SUCCESS;


copy_to_usr_err:
get_resp_vaddr_error:
    (void)_sp_get_id_name_batch_free_sharedmem(resp_offset_in_sharedmem);
free_sharedmem_err:
    return status;
}




//#include <linux/socket.h>
//#include <linux/net.h>
//#include <net/sock.h>
//#include <linux/in.h>

/*static process_id current_pid = 0;
static bool new_request_in_app = false;
void sa_connect_pid(process_id pid)
{
	current_pid = pid;
}

void sa_unconnect_pid(process_id pid)
{
	current_pid = 0;
}

// Push a request to App
void sa_app_push_request(char* app_msg_data, size_t app_msg_size)
{	
	// Step 1. Transmit the data to the App.
	comm_app_enqueue(current_pid, app_msg_data, app_msg_size);

	new_request_in_app = true;
}

//! Notify the App
void sa_app_notify(void)
{
	if(new_request_in_app)
		os_notify_app(current_pid);

	new_request_in_app = false;
}*/