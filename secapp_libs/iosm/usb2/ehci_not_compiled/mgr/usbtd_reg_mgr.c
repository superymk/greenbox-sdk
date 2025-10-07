// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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