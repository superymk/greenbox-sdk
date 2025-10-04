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

#ifndef LIBDRV_HID_USB2_KBD_H
#define LIBDRV_HID_USB2_KBD_H

#include <palddk.h>
// #include <iosm_usb2_sp_intfs.h>
#include <lib-iosm-usb.h>
#include <libdrv_hid_headers.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS usb_kbd_mgr_init(void);
extern void usb_kbd_mgr_fini(void);

extern struct sp_usbpdev_pipe* usb_kbd_init(struct sp_usbpdev_s* usbkbd_dev, struct usb_interface_descriptor *iface);
extern void usb_kbd_fini(struct sp_usbpdev_pipe* pipe);

//! \brief Read kbd event due to periodically pull and/or kbd interrupts
extern SAP_STATUS usb_kbd_handler(void);

// //! \brief Register a callback function to handle mouse events
// extern void usb2_kbd_register_listener(KBD_LISTENER listener);

//! Read keyboard input. The function blocks if there is no key pressed/released.
extern SAP_STATUS usb_kbd_read(struct kbd_key_event* out_key_event);

extern SAP_STATUS usb_kbd_read_nonblock(struct kbd_key_event* out_1st_key_event, struct kbd_key_event* out_2nd_key_event);

// extern void kbd_isr(void (*handler)(struct kbd_key_event* e));

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_USB2_KBD_H