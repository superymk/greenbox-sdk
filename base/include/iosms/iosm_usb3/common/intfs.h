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

#ifndef _IOSM_USB3_PUBLIC_COMMON_INTFS_H
#define _IOSM_USB3_PUBLIC_COMMON_INTFS_H


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
#endif //_IOSM_USB3_PUBLIC_COMMON_INTFS_H