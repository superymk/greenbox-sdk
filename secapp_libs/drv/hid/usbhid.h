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

/// @brief Read the HID descriptor content of the given endpoint.
/// @param dev 
/// @param ep_desc 
/// @param out_hid_desc_content 
/// @return 
extern SAP_STATUS read_hid_desc_content(struct sp_usbpdev_s* dev, struct usbpdev_endpoint_descriptor* ep_desc, uint8_t* out_hid_desc_content);

// Send USB HID protocol message.
extern SAP_STATUS set_protocol(struct sp_usbpdev_s* dev, u16 val, u16 inferface);

// Send USB HID SetIdle request.
extern SAP_STATUS set_idle(struct sp_usbpdev_s* dev, int ms);

extern SAP_STATUS usbhid_init_dev(struct sp_usbpdev_s* dev);
extern SAP_STATUS usbhid_fini_dev(struct sp_usbpdev_s* dev);

extern SAP_STATUS usb_intr_handler(intr_t intr);

/********* Debug functions *********/
extern void usbhid_print_hid_desc(struct usb_hid_descriptor *hid_desc);

#endif // __ASSEMBLY__
#endif // _SP_USB_HID_H