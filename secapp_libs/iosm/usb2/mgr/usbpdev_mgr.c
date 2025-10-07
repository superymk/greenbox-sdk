// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include "../lib-iosm-usb-internal.h"


/// @brief Map <USBPDev slot ID, USBPDev addr>
static USBPDEV_ADDR _usbpdev_slot_map[USBPDEV_SLOTS];



static void _usbpdev_slot_map_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, USBPDEV_SLOTS, USBPDEV_SLOTS, 0, 1)
    {
        usbpdev_addr_copy(&_usbpdev_slot_map[i], &USBADDR_INVALID);
    }
}




/********* Public functions *********/
void usbpdev_mgr_init(void)
{
    _usbpdev_slot_map_clean();
}

void usbpdev_mgr_fini(void)
{
    _usbpdev_slot_map_clean();
}

SAP_STATUS usbpdev_register(uint32_t slot, USBPDEV_ADDR addr)
{
    // Sanity checks
    if(!VALUE_IN_RANGE(slot, 0, USBPDEV_SLOTS))
        return EINVAL;

    if(usbpdev_addr_equal(&addr, &USBADDR_INVALID))
        return EINVAL;

    usbpdev_addr_copy(&_usbpdev_slot_map[slot], &addr);

    return SUCCESS;
}

SAP_STATUS usbpdev_unregister(uint32_t slot)
{
    // Sanity checks
    if(!VALUE_IN_RANGE(slot, 0, USBPDEV_SLOTS))
        return EINVAL;

    usbpdev_addr_copy(&_usbpdev_slot_map[slot], &USBADDR_INVALID);

    return SUCCESS;
}

/// @brief Return the slot that stores <addr>. Return -1 if not found.
uint32_t usbpdev_get_slot(USBPDEV_ADDR addr)
{
    uint32_t i = 0;

    FOREACH_S(i, USBPDEV_SLOTS, USBPDEV_SLOTS, 0, 1)
    {
        if(usbpdev_addr_equal(&_usbpdev_slot_map[i], &addr))
            // Found
            return i;
    }

    return INVALID_SLOT;
}