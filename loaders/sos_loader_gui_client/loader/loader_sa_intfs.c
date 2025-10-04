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

#include <loader_sa_intfs.h>

#include <stdio.h>

#include "../gui/util_log.h"

extern bool os_ioctl(struct tz_cmd_packet* cmd, int* err_no);

OSSTATUS secagent_nsdom_share_data_to_any_ns_process_set(uint64_t selector, uint64_t data)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET;
	cmd.arg1 = selector;
	cmd.arg2 = data;
	status = os_ioctl(&cmd, &ret);
	if(!status)
		FileUtils_Log("<secagent_nsdom_share_data_to_any_ns_process_set> error with code %d!\n", ret);

	return (OSSTATUS)cmd.status;
}

OSSTATUS secagent_nsdom_share_data_to_any_ns_process_get(uint64_t selector, uint64_t* data)
{
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;

	if(!data)
		return E_INV_ARG;
	
	cmd.id = CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_GET;
	cmd.arg1 = selector;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		FileUtils_Log("<secagent_nsdom_share_data_to_any_ns_process_get> error with code %d!\n", ret);
	}

	*data = cmd.arg2;
	return (OSSTATUS)cmd.status;
}