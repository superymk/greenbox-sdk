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

#include "util.h"

/// @brief Definition of Invalid addr of USBPDEV.
/// The definition must be consistent with <iosm_usb3.c> of IOSM-USB2 and IOSM-USB3
USBPDEV_ADDR USBADDR_INVALID = {
	.dev_usbaddr = 0,
	.hub_port = 0,
	.hub_usbaddr = 0,
	.bus_id = 0,
	.reserved1 = 0,
	.reserved2 = 0
};

static inline u64 rdtscll(void)
{
    u64 val;
    asm volatile("rdtsc" : "=A" (val));
    return val;
}

// Sample the current timer value.
uint64_t timer_read(void)
{
    return rdtscll();
}

bool usbpdev_addr_equal(USBPDEV_ADDR* addr1, USBPDEV_ADDR* addr2)
{
    if((addr1->v.low == addr2->v.low) && (addr1->v.high == addr2->v.high))
        return true;
    else
        return false;
}

void usbpdev_addr_copy(USBPDEV_ADDR* to, USBPDEV_ADDR* from)
{
    to->v.low = from->v.low;
    to->v.high = from->v.high;
}