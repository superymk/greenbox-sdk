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

#include <sa_headers.h>


OSSTATUS do_CMD_SECBASE_INIT_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet)
{
    ulong_t ret = 0;

	UNUSED(cmd_packet);
	
    ret = os->hypercall_0arg(NS_HC_INIT_SECBASE_ONDEMAND_PER_CPU);
    if (ret)
    {
        DRV_ERR("Failed to init SecBase on-demand of the current CPU!\n");
        return E_INTERNAL_ERROR;
    }

    return OS_SUCCESS;
}

OSSTATUS do_CMD_SECBASE_FINI_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet)
{
    ulong_t ret = 0;

	UNUSED(cmd_packet);
	
    ret = os->hypercall_0arg(NS_HC_FINI_SECBASE_ONDEMAND_PER_CPU);
    if (ret)
    {
        DRV_ERR("Failed to finalize SecBase on-demand of the current CPU!\n");
        return E_INTERNAL_ERROR;
    }

    return OS_SUCCESS;
}