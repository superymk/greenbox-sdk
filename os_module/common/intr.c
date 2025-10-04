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