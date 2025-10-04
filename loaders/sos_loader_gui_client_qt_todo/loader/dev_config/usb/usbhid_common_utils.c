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

// #include <loader_sa_intfs.h>

#include <dev_config/usbhid_probe.h>
// #include <sl_headers.h>

#include <stdio.h>

bool usbpdev_addr_equal(USBPDEV_ADDR* addr1, USBPDEV_ADDR* addr2)
{
    if((addr1->v == addr2->v))
        return true;
    else
        return false;
}

void usbpdev_addr_copy(USBPDEV_ADDR* to, USBPDEV_ADDR* from)
{
    to->v = from->v;
}