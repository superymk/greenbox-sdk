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

#include <palddk.h>
#include "../lib-iosm-usb-internal.h"


/// @brief Map <USBTD Reg index, bool>
/// bool: "allocated or not"
static bool _usbtd_reg_alloc_map[USBTD_REG_NUMS];

static void _usbtd_reg_alloc_map_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, USBTD_REG_NUMS, USBTD_REG_NUMS, 0, 1)
    {
        _usbtd_reg_alloc_map[i] = false;
    }
}




/********* Public functions *********/
void usbtd_reg_mgr_init(void)
{
    _usbtd_reg_alloc_map_clean();
}

void usbtd_reg_mgr_fini(void)
{
    _usbtd_reg_alloc_map_clean();
}

usbtd_reg_slot_t usbtd_reg_pick(void)
{
    uint32_t i = 0;

    FOREACH_S(i, USBTD_REG_NUMS, USBTD_REG_NUMS, 0, 1)
    {
        if(_usbtd_reg_alloc_map[i] == false)
        {
            _usbtd_reg_alloc_map[i] = true;
            return i;
        }
    }

    return USBTD_REG_SLOT_INVALID;
}

void usbtd_reg_return(usbtd_reg_slot_t reg_slot)
{
    if(IS_USBTD_REG_SLOT_VALID(reg_slot))
    {
        _usbtd_reg_alloc_map[reg_slot] = false;
    }
}