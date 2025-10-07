// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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