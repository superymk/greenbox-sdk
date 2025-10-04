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

struct eehci_info_entry
{
    union eEHCI_ID  eehci_id;
    uint32_t        eehci_handle;
};


/// @brief Map <USBPDev slot ID, USBPDev addr>
static struct eehci_info_entry _eehci_slot_map[EEHCI_SLOTS];

static void _eehci_slot_map_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, EEHCI_SLOTS, EEHCI_SLOTS, 0, 1)
    {
        _eehci_slot_map[i].eehci_id.v = eEHCI_ID_INVALID;
        _eehci_slot_map[i].eehci_handle = eEHCI_Handle_INVALID;
    }
}




/********* Public functions *********/
void eehci_mgr_init(void)
{
    _eehci_slot_map_clean();
}

void eehci_mgr_fini(void)
{
    _eehci_slot_map_clean();
}

SAP_STATUS eehci_register(uint32_t slot, union eEHCI_ID eehci_id, uint32_t eehci_handle)
{
    // Sanity checks
    if(!VALUE_IN_RANGE(slot, 0, EEHCI_SLOTS))
        return EINVAL;

    if((eehci_id.v == eEHCI_ID_INVALID) || (eehci_handle == eEHCI_Handle_INVALID))
        return EINVAL;

    _eehci_slot_map[slot].eehci_id.v = eehci_id.v;
    _eehci_slot_map[slot].eehci_handle = eehci_handle;

    return SUCCESS;
}

SAP_STATUS eehci_unregister(uint32_t slot)
{
    // Sanity checks
    if(!VALUE_IN_RANGE(slot, 0, EEHCI_SLOTS))
        return EINVAL;

    _eehci_slot_map[slot].eehci_id.v = eEHCI_ID_INVALID;
    _eehci_slot_map[slot].eehci_handle = eEHCI_Handle_INVALID;

    return SUCCESS;
}

/// @brief Return the eEHCI info stored at slot <slot>
SAP_STATUS eehci_get_info(uint32_t slot, union eEHCI_ID* out_eehci_id, uint32_t* out_eehci_handle)
{
    // Sanity checks
    if(!VALUE_IN_RANGE(slot, 0, EEHCI_SLOTS))
        return EINVAL;

    if(!out_eehci_id || !out_eehci_handle)
        return EINVAL;

    (*out_eehci_id).v = _eehci_slot_map[slot].eehci_id.v;
    *out_eehci_handle = _eehci_slot_map[slot].eehci_handle;

    return SUCCESS;
}