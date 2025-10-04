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

/********* Data *********/
static COMM_ISP_SHAREDMEM* comm_dtm = NULL;
static spid_t dtm_spid = INVALID_SP_ID;




/********* Private Functions *********/
static bool _is_response_success(DTM_ISP_MSG* msg)
{
    // Block waiting until the request is handled. This is necessary because DesktopMgr may be preempted in the middle 
    // of handling requests.
    // [NOTE] Clients' communications with IOSMs do not need this block waiting, because SecOS ALWAYS disables all 
    // interrupts before invoking IOSMs to handle client SecProcess's requests.
    while(!msg->header.is_response)
    {
        sp_yield();
    }

    // Check response status
    if((msg->response.status1 == DTM_STATUS1_SUCCESS))
        return true;
    
    return false;
}

/// @brief Clear the communication buffer to prevent new requests from reading old responses.
static void _clear_comm_buf(void)
{
    DTM_ISP_MSG* dtm_msg = (DTM_ISP_MSG*)comm_dtm->comm_buf;

    memset(dtm_msg, 0, sizeof(DTM_ISP_MSG));
}




/********* Public Functions *********/
/// @brief Create Inter-SecProcess communication channels to DesktopMgr.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
SAP_STATUS comm_dtm_init(spid_t in_dtm_spid)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE* dtm_msg = NULL;

    // Create communication channel with the DesktopMgr service.
	comm_dtm = comm_isp_sharedmem_create(in_dtm_spid, SECSRV_DTM_INCOMMING_COMM_ISP_PAGES);
    if(!comm_dtm)
    {
        SAP_ERR("Failed to create Inter-SecProcess communication channel to DesktopMgr (id:%u)!\n", in_dtm_spid);
        return EINTERNAL;
    }

    status = comm_isp_sharedmem_register(comm_dtm);
    if(FAIL(status))
    {
        SAP_ERR("Failed to register Inter-SecProcess communication channel with DesktopMgr (id:%u)! status:%lu\n", in_dtm_spid, status);
        return EINTERNAL;
    }

    // Switch to the DesktopMgr service to setup the channel.
    sos_sys_switch_to_secprocess_event_handling(in_dtm_spid);
    
    // Check the reponse 
    dtm_msg = (COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE*)comm_dtm->comm_buf;
    if((dtm_msg->is_response) && (dtm_msg->status == COMM_ISP_SHAREDMEM_STATUS_ESTABLISHED))
    {
        dtm_spid = in_dtm_spid;
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

void comm_dtm_fini(void)
{
    if(comm_dtm)
    {
        comm_isp_sharedmem_unregister_and_close(comm_dtm);

        dtm_spid = INVALID_SP_ID;
        comm_dtm = NULL;
    }
}

SAP_STATUS dtm_register_window(uint32_t window_init_width, uint32_t window_init_height, shandle_t* out_h_fb, size_t* out_fb_size, uint32_t* out_fb_max_width, uint32_t* out_fb_max_height, shandle_t* out_h_rb_ctl, shandle_t* out_h_rb_buf)
{
    SAP_STATUS status = SUCCESS;
    DTM_ISP_MSG* dtm_msg = NULL;
    DTM_REQUEST_CREATE_WINDOW params;
    DTM_RESPONSE_CREATE_WINDOW* resp_data = NULL;

    // Sanity checks
    if(!comm_dtm)
        return EINVAL;

    if(dtm_spid == INVALID_SP_ID)
        return EINVAL;

    if(!out_h_fb || !out_fb_size || !out_fb_max_width || !out_fb_max_height || !out_h_rb_ctl || !out_h_rb_buf)
        return EINVAL;

    // Clear communication buffer is necessary, or new requests in the next may read old responses.
    _clear_comm_buf();

    // Call DesktopMgr
    params.window_init_width = window_init_width;
    params.window_init_height = window_init_height;
    status = comm_isp_msg_call(dtm_spid, false, SECSRV_DTM_SUBCMD_CREATE_WINDOW, &params, sizeof(DTM_REQUEST_CREATE_WINDOW), NULL, NULL);
    if(FAIL(status))
    {
        SAP_ERR("Failed to send the request SECSRV_DTM_SUBCMD_CREATE_WINDOW to DesktopMgr (id:%u)! status:%lu\n", dtm_spid, status);
        return EINTERNAL;
    }

    // Check the response
    dtm_msg = (DTM_ISP_MSG*)comm_dtm->comm_buf;
    if(!_is_response_success(dtm_msg))
        return EINTERNAL;
        
    // On success
    resp_data = (DTM_RESPONSE_CREATE_WINDOW*)dtm_msg->response.data;
    *out_h_fb = resp_data->h_fb;
    *out_fb_size = resp_data->fb_size;
    *out_fb_max_width = resp_data->fb_max_width;
    *out_fb_max_height = resp_data->fb_max_height;

    *out_h_rb_ctl = resp_data->h_rb_ctl;
	*out_h_rb_buf = resp_data->h_rb_buf;

    return SUCCESS;
}

SAP_STATUS dtm_unregister_window(void)
{
    SAP_STATUS status = SUCCESS;
    DTM_ISP_MSG* dtm_msg = NULL;
    bool is_resp_success = false;

    // Sanity checks
    if(!comm_dtm)
        return EINVAL;

    if(dtm_spid == INVALID_SP_ID)
        return EINVAL;

    // Clear communication buffer is necessary, or new requests in the next may read old responses.
    _clear_comm_buf();

    // Call DesktopMgr
    status = comm_isp_msg_call(dtm_spid, false, SECSRV_DTM_SUBCMD_DESTROY_WINDOW, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        SAP_ERR("Failed to send the request SECSRV_DTM_SUBCMD_DESTROY_WINDOW to DesktopMgr (id:%u)! status:%lu\n", dtm_spid, status);
        return EINTERNAL;
    }

    // Check the response
    dtm_msg = (DTM_ISP_MSG*)comm_dtm->comm_buf;
    is_resp_success = _is_response_success(dtm_msg);

    // Return result
    if(is_resp_success)
        return SUCCESS;
    else
        return EINTERNAL;
}