// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

OSSTATUS do_CMD_OS_TRIGGER_INTR(struct tz_cmd_packet *cmd_packet)
{
    uint32_t intr = 0;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;

    intr = (uint32_t)cmd_packet->arg1;
    os->intr_trigger_interrupt(intr);

    return OS_SUCCESS;
}