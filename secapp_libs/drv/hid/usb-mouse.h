// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _SP_USB_MOUSE_DRIVER_H
#define _SP_USB_MOUSE_DRIVER_H

#include <palddk.h>
#include <lib-iosm-usb.h>
#include <libdrv_hid_headers.h>

#ifndef __ASSEMBLY__

struct hid_report_descriptor;

extern SAP_STATUS usb_mouse_mgr_init(void);
extern void usb_mouse_mgr_fini(void);

/* Prepare the mouse interface, register its pipe, and retain its descriptor. */
extern struct sp_usbpdev_pipe *usb_mouse_init(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor);
/* Takes ownership of descriptor on both success and failure. */
extern struct sp_usbpdev_pipe *usb_mouse_init_from_descriptor(struct sp_usbpdev_s *device,
    struct usbpdev_endpoint_descriptor *endpoint, struct hid_report_descriptor *descriptor,
    bool boot_protocol);
extern void usb_mouse_fini(struct sp_usbpdev_pipe *interrupt_pipe);

/* Initialize out_event and return at most one event without waiting for input. */
extern SAP_STATUS usb_mouse_read_nonblock(struct mouse_event *out_event);

//! \brief Read mouse event due to periodically pull and/or mouse interrupts
/* Deliver the current report's queued events to listeners before polling again. */
extern SAP_STATUS usb_mouse_handler(void);

//! \brief Register a callback function to handle mouse events
extern void usb2_mouse_register_listener(MOUSE_LISTENER listener);

#endif // __ASSEMBLY__
#endif // _SP_USB_MOUSE_DRIVER_H
