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

#include "iosm_ps2_client.h"
#include "iosm_ps2_calls.h"


/********* Private functions *********/
static SAP_STATUS _iosm_ps2_client_init(spid_t iosm_ps2_spid)
{
    SAP_STATUS status = SUCCESS;
	
	// Sanity checks
	if(iosm_ps2_spid == INVALID_SP_ID)
		return EINVAL;

    // Create communication channel with the IOSM-PS2 service.
	status = comm_iosm_ps2_init(iosm_ps2_spid);
	if(FAIL(status))
	{
		SAP_ERR("Init Lib-PS2 error: Failed to init Inter-SecProcess communication channel with IOSM-PS2 (id:%u)! status:%lu\n", iosm_ps2_spid, status);
        status = EINTERNAL;
		goto init_comm_err;
	}

	// Create PS2 I/O partition and register PS2 driver
	status = iosm_ps2_register_ps2drv();
	if(FAIL(status))
	{
		SAP_ERR("Init Lib-PS2 error: Failed to register PS2 driver with IOSM-PS2 (id:%u)! status:%lu\n", iosm_ps2_spid, status);
        status = EINTERNAL;
		goto register_ps2drv_partition_err;
	}

	SAP_INFO("Initial Lib-PS2 Success: Created PS2 I/O partition\n");
	return SUCCESS;


register_ps2drv_partition_err:
	comm_iosm_ps2_fini();
init_comm_err:
    return status;
}

static void _iosm_ps2_client_fini(void)
{
	SAP_STATUS status = SUCCESS;

	// Unregister PS2 driver and destroy partition
	status = iosm_ps2_unregister_ps2drv();
	if(FAIL(status))
		SAP_ERR("Finalize Lib-PS2 error: Failed to unregister PS2 driver with IOSM-PS2! status:%lu\n", status);

    // Destroy communication channel with the IOSM-USB2 service.
	comm_iosm_ps2_fini();
}




/********* Public functions *********/
SAP_STATUS init_iosm_ps2_client(void)
{
    SAP_STATUS status = SUCCESS;
    spid_t iosm_ps2_spid = INVALID_SP_ID;

    // Get the SecProcess ID of IOSM-PS2
    iosm_ps2_spid = sos_sys_sp_query_id(SP_SERVICE, "IOSM-PS2", true);
    if(iosm_ps2_spid == INVALID_SP_ID)
    {
        SAP_ERR("Failed to get the SecProcess ID of IOSM-PS2!\n");
        return EINTERNAL;
    }

    // Internal init
	status = _iosm_ps2_client_init(iosm_ps2_spid);
	if(FAIL(status))
	{
		SAP_ERR("Init Lib-PS2 error! status:%lu\n", status);
		return EINTERNAL;
	}

    return SUCCESS;
}

void fini_libps2_client(void)
{
    _iosm_ps2_client_fini();
}