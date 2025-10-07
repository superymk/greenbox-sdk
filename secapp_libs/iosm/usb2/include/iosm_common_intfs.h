// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file Copied from iosm_usb2/include/public/common/intfs.h (same with the file in iosm_usb3)

#ifndef _IOSM_USB_PUBLIC_COMMON_INTFS_H
#define _IOSM_USB_PUBLIC_COMMON_INTFS_H


#ifndef __ASSEMBLY__

#ifndef IOSM_USB_USBPDEV_ADDR
#define IOSM_USB_USBPDEV_ADDR 1
typedef union usbpdev_addr
{
    struct {
        unsigned int dev_usbaddr:7,
                    hub_port:7,
                    hub_usbaddr:7,
                    reserved2:11;
        unsigned int bus_id:16,
                    reserved1:16;
    };
    QWORD v;
} USBPDEV_ADDR;

#endif // IOSM_USB_USBPDEV_ADDR

/// @brief Definition of Invalid addr of USBPDEV.
extern USBPDEV_ADDR USBADDR_INVALID;

#endif// __ASSEMBLY__
#endif //_IOSM_USB_PUBLIC_COMMON_INTFS_H