// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef GREENBOX_USE_UXMHF64_V1

#include <sa_headers.h>

extern void os_send_ipi_to_cpu(uint8_t vector, lapic_id_t apic_id);

static uint32_t tos_lapic_id = 0;
static uint32_t tos_ipi_vector = 0;

OSSTATUS sb_get_tos_info(NS_HC_RESP_GET_TINYOS_INFO *tos_info)
{
    ulong_t ret = 0;

    ret = os->hypercall_1arg(NS_HC_GET_TINYOS_INFO, (vaddr_t)tos_info);
    if (ret)
    {
        DRV_ERR("Failed to get TinyOS's info! ret:%lu\n", ret);
        return E_INTERNAL_ERROR;
    }

    // On success
    return OS_SUCCESS;
}

OSSTATUS tos_probe_configs(bool *out_is_running)
{
    OSSTATUS status = OS_SUCCESS;
    NS_HC_RESP_GET_TINYOS_INFO tos_info = {0};

    status = sb_get_tos_info(&tos_info);
    if (SA_FAIL(status))
    {
        DRV_ERR("Failed to get TinyOS's running status! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

    tos_lapic_id = tos_info.tos_lapic_id;
    tos_ipi_vector = tos_info.tos_ipi_vector;
    *out_is_running = (tos_info.running_status == TINYOS_STATUS_RUNNING);

    // On success
    return OS_SUCCESS;
}

OSSTATUS tos_get_running_status(bool *out_is_running)
{
    OSSTATUS status = OS_SUCCESS;
    NS_HC_RESP_GET_TINYOS_INFO tos_info = {0};

    status = sb_get_tos_info(&tos_info);
    if (SA_FAIL(status))
    {
        DRV_ERR("Failed to get TinyOS's running status! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

    *out_is_running = (tos_info.running_status == TINYOS_STATUS_RUNNING);

    // On success
    return OS_SUCCESS;
}

OSSTATUS tos_notify_ipi(void)
{
    if(!tos_ipi_vector)
    {
        DRV_ERR("Invalid TinyOS IPI vector!\n");
        return E_INV_ARG;
    }

    os_send_ipi_to_cpu((uint8_t)tos_ipi_vector, tos_lapic_id);

    // On success
    return OS_SUCCESS;
}

#endif // !GREENBOX_USE_UXMHF64_V1