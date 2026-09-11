// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBDRV_HID_USB2_KBD_H
#define LIBDRV_HID_USB2_KBD_H

#include <palddk.h>
#include <lib-iosm-usb.h>
#include <libdrv_hid_headers.h>

#ifndef __ASSEMBLY__

struct hid_report_descriptor;

extern SAP_STATUS usb_kbd_mgr_init(void);
extern void usb_kbd_mgr_fini(void);

/* Prepare the keyboard interface, register its pipe, and retain its descriptor. */
extern struct sp_usbpdev_pipe *usb_kbd_init(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor);
/* Takes ownership of descriptor on both success and failure. */
extern struct sp_usbpdev_pipe *usb_kbd_init_from_descriptor(struct sp_usbpdev_s *device,
    struct usbpdev_endpoint_descriptor *endpoint, struct hid_report_descriptor *descriptor,
    bool boot_protocol);
extern void usb_kbd_fini(struct sp_usbpdev_pipe *interrupt_pipe);

//! \brief Read kbd event due to periodically pull and/or kbd interrupts
/* Deliver every remaining transition from the current report to listeners. */
extern SAP_STATUS usb_kbd_handler(void);

//! Read keyboard input. The function blocks if there is no key pressed/released.
/* Additional queued transitions remain available to the next read. */
extern SAP_STATUS usb_kbd_read(struct kbd_key_event *out_key_event);

/* Both outputs are initialized on every call; remaining events stay queued. */
extern SAP_STATUS usb_kbd_read_nonblock(struct kbd_key_event *out_1st_key_event,
    struct kbd_key_event *out_2nd_key_event);

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_USB2_KBD_H
