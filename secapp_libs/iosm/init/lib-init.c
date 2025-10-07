// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include <lib_init_headers.h>

#include "secsrv_init_calls.h"

/********* Public Functions *********/
SAP_STATUS secsrv_init_init(spid_t secsrv_init_spid)
{
	SAP_STATUS status = SUCCESS;

	// Sanity checks
	if (secsrv_init_spid == INVALID_SP_ID)
		return EINVAL;

	// Create communication channel with the SECSRV-INIT service.
	status = comm_secsrv_init_init(secsrv_init_spid);
	if (FAIL(status))
	{
		INIT_ERR("Init Lib-INIT error: Failed to init Inter-SecProcess communication channel with SECSRV-INIT (id:%u)! status:%lu\n", secsrv_init_spid, status);
		status = EINTERNAL;
		goto init_comm_err;
	}

	// On success
	return SUCCESS;

init_comm_err:
	return status;
}

void secsrv_init_fini(void)
{
	// Destroy communication channel with the IOSM-USB2 service.
	comm_secsrv_init_fini();
}
