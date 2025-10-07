// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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