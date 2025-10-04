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

#ifndef _SP_USB_HID_H
#define _SP_USB_HID_H

#include <palddk.h>
#include <lib-iosm-usb.h>

#define SP_USBDRV_USBBUF_4K_PAGES       (128)

#define USBHID_INTF_PROTOCOL_KBD        (0x1)
#define USBHID_INTF_PROTOCOL_MOUSE      (0x2)

#define E_HID_NOT_KBD_OR_MOUSE          (1100)

#ifndef __ASSEMBLY__

extern SAP_STATUS usbhid_mgr_init(void);
extern void usbhid_mgr_fini(void);

// Send USB HID protocol message.
extern SAP_STATUS set_protocol(struct sp_usbpdev_s* dev, u16 val, u16 inferface);

// Send USB HID SetIdle request.
extern SAP_STATUS set_idle(struct sp_usbpdev_s* dev, int ms);

extern SAP_STATUS usbhid_init_dev(struct sp_usbpdev_s* dev);
extern SAP_STATUS usbhid_fini_dev(struct sp_usbpdev_s* dev);

extern SAP_STATUS usb_intr_handler(intr_t intr);

#endif // __ASSEMBLY__
#endif // _SP_USB_HID_H