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

#include <palddk.h>
#include <secsrv_init_sp_intfs.h>
#include <lib_init_headers.h>
#include "secsrv_init_calls.h"

/********* Data *********/
static COMM_ISP_SHAREDMEM* comm_secsrv_init = NULL;
static spid_t secsrv_init_spid = INVALID_SP_ID;




/********* Public Functions *********/
/// @brief Create Inter-SecProcess communication channels required by SECSRV-INIT.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
SAP_STATUS comm_secsrv_init_init(spid_t in_secsrv_init_spid)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE* secsrv_init_msg = NULL;

    // Create communication channel with the SECSRV-INIT service.
	comm_secsrv_init = comm_isp_sharedmem_create(in_secsrv_init_spid, SECSRV_INIT_INCOMMING_COMM_ISP_PAGES);
    if(!comm_secsrv_init)
    {
        INIT_ERR("Failed to create Inter-SecProcess communication channel with SECSRV-INIT (id:%u)!\n", in_secsrv_init_spid);
        return EINTERNAL;
    }

    status = comm_isp_sharedmem_register(comm_secsrv_init);
    if(FAIL(status))
    {
        INIT_ERR("Failed to register Inter-SecProcess communication channel with SECSRV-INIT (id:%u)! status:%lu\n", in_secsrv_init_spid, status);
        return EINTERNAL;
    }

    // Switch to the SECSRV-INIT service to setup the channel.
    sos_sys_switch_to_secprocess_event_handling(in_secsrv_init_spid);
    
    // Check the reponse 
    secsrv_init_msg = (COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE*)comm_secsrv_init->comm_buf;
    if((secsrv_init_msg->is_response) && (secsrv_init_msg->status == COMM_ISP_SHAREDMEM_STATUS_ESTABLISHED))
    {
        secsrv_init_spid = in_secsrv_init_spid;
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

void comm_secsrv_init_fini(void)
{
    if(comm_secsrv_init)
    {
        comm_isp_sharedmem_unregister_and_close(comm_secsrv_init);
        comm_secsrv_init = NULL;
    }
}

static bool _is_response_success(SECSRV_INIT_ISP_MSG* msg)
{
    if((msg->header.is_response) && (msg->response.status1 == SECSRV_INIT_STATUS1_SUCCESS))
        return true;
    
    return false;
}




/********* Operations of SECSRV-INIT *********/
SAP_STATUS secsrv_init_sp_priority_update(spid_t target_sp_id, sp_priority_t new_priority)
{
    SAP_STATUS status = SUCCESS;
    SECSRV_INIT_ISP_MSG* secsrv_init_msg = NULL;
    SECSRV_INIT_SP_PRIORITY_UPDATE_PARAMS params;

    // Sanity checks
    if(!comm_secsrv_init)
        return EINVAL;

    if(secsrv_init_spid == INVALID_SP_ID)
        return EINVAL;

    // Call SECSRV-INIT
    params.target_sp_id = target_sp_id;
    params.new_priority = new_priority;
    status = comm_isp_msg_call(secsrv_init_spid, false, SECSRV_INIT_SECPROCESS_PRIORITY_UPDATE, &params, sizeof(SECSRV_INIT_SP_PRIORITY_UPDATE_PARAMS), NULL, NULL);
    if(FAIL(status))
    {
        INIT_ERR("[SECSRV-INIT (id:%u)] Failed to send the request SECSRV_INIT_SECPROCESS_PRIORITY_UPDATE! status:%lu\n", secsrv_init_spid, status);
        status = EINTERNAL;
        goto call_secsrv_init_err;
    }

    // Check the response
    secsrv_init_msg = (SECSRV_INIT_ISP_MSG*)comm_secsrv_init->comm_buf;
    if(!_is_response_success(secsrv_init_msg))
    {
        status = EINTERNAL;
        goto secsrv_init_resp_err;
    }
    
    // On success
    return SUCCESS;
    
secsrv_init_resp_err:
call_secsrv_init_err:
    return status;
}