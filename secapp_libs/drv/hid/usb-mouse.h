// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _SP_USB_MOUSE_DRIVER_H
#define _SP_USB_MOUSE_DRIVER_H

#include <palddk.h>
// #include <iosm_usb2_sp_intfs.h>
#include <lib-iosm-usb.h>
#include <libdrv_hid_headers.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS usb_mouse_mgr_init(void);
extern void usb_mouse_mgr_fini(void);

extern struct sp_usbpdev_pipe* usb_mouse_init(struct sp_usbpdev_s* usbmouse_dev, struct usb_interface_descriptor *iface);
extern void usb_mouse_fini(struct sp_usbpdev_pipe* pipe);

extern SAP_STATUS usb_mouse_read_nonblock(struct mouse_event* out_1st_event);

//! \brief Read mouse event due to periodically pull and/or mouse interrupts
extern SAP_STATUS usb_mouse_handler(void);

//! \brief Register a callback function to handle mouse events
extern void usb2_mouse_register_listener(MOUSE_LISTENER listener);

#endif // __ASSEMBLY__
#endif // _SP_USB_MOUSE_DRIVER_H