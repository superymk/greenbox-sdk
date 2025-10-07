// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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