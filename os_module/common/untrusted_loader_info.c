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

/// @brief Data store for <CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS>
static uint64_t any_nsproc_info_store[NS_MAX_INFO_SELECTOR] = {0};

/********* Functions for CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS *********/
OSSTATUS do_CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET(struct tz_cmd_packet *cmd_packet)
{
    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;

    if(!VALUE_IN_RANGE(cmd_packet->arg1, 0, NS_MAX_INFO_SELECTOR))
        return E_INV_ARG;

    any_nsproc_info_store[cmd_packet->arg1] = cmd_packet->arg2;

    // On success
    return OS_SUCCESS;
}

OSSTATUS do_CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_GET(struct tz_cmd_packet *cmd_packet)
{
    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;

    if(!VALUE_IN_RANGE(cmd_packet->arg1, 0, NS_MAX_INFO_SELECTOR))
        return E_INV_ARG;

    cmd_packet->arg2 = any_nsproc_info_store[cmd_packet->arg1];

    // On success
    return OS_SUCCESS;
}