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

#include <stdio.h>
#include "iosm_ps2_calls.h"

/********* Data *********/
static COMM_ISP_SHAREDMEM* comm_iosm_ps2 = NULL;
static spid_t iosm_ps2_spid = INVALID_SP_ID;




/********* Public Functions *********/
/// @brief Create Inter-SecProcess communication channels required by IOSM-PS2.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
SAP_STATUS comm_iosm_ps2_init(spid_t in_iosm_ps2_spid)
{
    SAP_STATUS status = SUCCESS;
    COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE* iosm_ps2_msg = NULL;

    // Create communication channel with the IOSM-PS2 service.
	comm_iosm_ps2 = comm_isp_sharedmem_create(in_iosm_ps2_spid, IOSM_PS2_INCOMMING_COMM_ISP_PAGES);
    if(!comm_iosm_ps2)
    {
        SAP_ERR("Failed to create Inter-SecProcess communication channel with IOSM-PS2 (id:%u)!\n", in_iosm_ps2_spid);
        return EINTERNAL;
    }

    status = comm_isp_sharedmem_register(comm_iosm_ps2);
    if(FAIL(status))
    {
        SAP_ERR("Failed to register Inter-SecProcess communication channel with IOSM-PS2 (id:%u)! status:%lu\n", in_iosm_ps2_spid, status);
        return EINTERNAL;
    }

    // Switch to the IOSM-PS2 service to setup the channel.
    sos_sys_switch_to_secprocess_event_handling(in_iosm_ps2_spid);
    
    // Check the reponse 
    iosm_ps2_msg = (COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE*)comm_iosm_ps2->comm_buf;
    if((iosm_ps2_msg->is_response) && (iosm_ps2_msg->status == COMM_ISP_SHAREDMEM_STATUS_ESTABLISHED))
    {
        iosm_ps2_spid = in_iosm_ps2_spid;
        return SUCCESS;
    }
    else
        return EINTERNAL;
}

void comm_iosm_ps2_fini(void)
{
    if(comm_iosm_ps2)
    {
        comm_isp_sharedmem_unregister_and_close(comm_iosm_ps2);
        comm_iosm_ps2 = NULL;
    }
}

static bool _is_response_success(IOSM_PS2_ISP_MSG* msg)
{
    if((msg->header.is_response) && (msg->response.status1 == IOSM_PS2_STATUS1_SUCCESS))
        return true;
    
    return false;
}




/********* Operations of IOSM-PS2 *********/
/// @brief Register the caller SecProcess's PS2 driver in IOSM-PS2.
SAP_STATUS iosm_ps2_register_ps2drv(void)
{
    SAP_STATUS status = SUCCESS;
    IOSM_PS2_ISP_MSG* iosm_ps2_msg = NULL;

    // Sanity checks
    if(!comm_iosm_ps2)
        return EINVAL;

    if(iosm_ps2_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-PS2
    status = comm_isp_msg_call(iosm_ps2_spid, false, IOSM_PS2_SUBCMD_REGISTER_USER, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        SAP_ERR("[IOSM-PS2 (id:%u)] Failed to send the request IOSM_PS2_SUBCMD_REGISTER_USER! status:%lu\n", iosm_ps2_spid, status);
        status = EINTERNAL;
        goto call_iosm_ps2_err;
    }

    // Check the response
    iosm_ps2_msg = (IOSM_PS2_ISP_MSG*)comm_iosm_ps2->comm_buf;
    if(_is_response_success(iosm_ps2_msg))
    {
        return SUCCESS;
    }
    else
        return EINTERNAL;


call_iosm_ps2_err:
    return status;
}

/// @brief Unregister the caller SecProcess's PS2 driver in IOSM-PS2.
SAP_STATUS iosm_ps2_unregister_ps2drv(void)
{
    SAP_STATUS status = SUCCESS;
    IOSM_PS2_ISP_MSG* iosm_ps2_msg = NULL;

    // Sanity checks
    if(!comm_iosm_ps2)
        return EINVAL;

    if(iosm_ps2_spid == INVALID_SP_ID)
        return EINVAL;

    // Call IOSM-PS2
    status = comm_isp_msg_call(iosm_ps2_spid, false, IOSM_PS2_SUBCMD_UNREGISTER_USER, NULL, 0, NULL, NULL);
    if(FAIL(status))
    {
        SAP_ERR("[IOSM-PS2 (id:%u)] Failed to send the request IOSM_PS2_SUBCMD_UNREGISTER_USER! status:%lu\n", iosm_ps2_spid, status);
        status = EINTERNAL;
        goto call_iosm_ps2_err;
    }

    // Check the response
    iosm_ps2_msg = (IOSM_PS2_ISP_MSG*)comm_iosm_ps2->comm_buf;
    if(_is_response_success(iosm_ps2_msg))
        return SUCCESS;
    else
        return EINTERNAL;


call_iosm_ps2_err:
    return status;
}