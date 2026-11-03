// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
#ifndef _SP_USB_HID_H
#define _SP_USB_HID_H

#include <palddk.h>
#include <lib-iosm-usb.h>

#define SP_USBDRV_USBBUF_4K_PAGES 128
#define USBHID_INTF_PROTOCOL_KBD 1
#define USBHID_INTF_PROTOCOL_MOUSE 2
#define E_HID_NOT_KBD_OR_MOUSE 1100
#define USBHID_PROTOCOL_BOOT 0
#define USBHID_PROTOCOL_REPORT 1

#ifndef __ASSEMBLY__
#include "hid-report.h"

/* Initialize or release the shared keyboard, mouse, and USB client resources. */
SAP_STATUS usbhid_mgr_init(void);
void usbhid_mgr_fini(void);
SAP_STATUS usbhid_init_dev(struct sp_usbpdev_s *device);
SAP_STATUS usbhid_fini_dev(struct sp_usbpdev_s *device);
SAP_STATUS usb_intr_handler(intr_t interrupt_number);

/// @brief Read the HID descriptor content of the given endpoint.
/// @param device USB device that owns the endpoint.
/// @param endpoint Endpoint metadata containing the HID descriptor and interface.
/// @param output_descriptor Caller-owned storage for the report descriptor bytes.
/// @param output_capacity Number of bytes available in output_descriptor.
/// @return SUCCESS after copying the descriptor; an error for invalid bounds,
/// allocation failure, or a failed control transfer.
SAP_STATUS read_hid_desc_content(struct sp_usbpdev_s *device,
    struct usbpdev_endpoint_descriptor *endpoint, uint8_t *output_descriptor,
    size_t output_capacity);
// Send USB HID protocol message.
/* selected_protocol is USBHID_PROTOCOL_REPORT or USBHID_PROTOCOL_BOOT.
 * interface_number identifies the target BOOT-subclass interface. */
SAP_STATUS set_protocol(struct sp_usbpdev_s *device, u16 selected_protocol,
    u16 interface_number);
// Send USB HID SetIdle request.
/* Duration zero requests reports only on changes, for all IDs on the interface. */
SAP_STATUS set_idle(struct sp_usbpdev_s *device, int milliseconds, u16 interface_number);

/* Parse descriptor data and select REPORT, or BOOT for an unsupported REPORT
 * layout on a compatible BOOT interface. Zero requested_input_type accepts a
 * keyboard or mouse; otherwise pass HID_REPORT_KEYBOARD or HID_REPORT_MOUSE.
 * On success the caller owns output_descriptor. BOOT uses a parsed standard
 * descriptor too, so both protocols use the same input decoding functions.
 * No pipe is allocated here; output_endpoint borrows the device's descriptor. */
SAP_STATUS usbhid_prepare_interface(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor, unsigned requested_input_type,
    struct usbpdev_endpoint_descriptor **output_endpoint,
    struct hid_report_descriptor **output_descriptor, bool *output_boot_protocol);
/* LED bits 0..4 select Num Lock, Caps Lock, Scroll Lock, Compose, and Kana. */
SAP_STATUS usbhid_send_leds(struct sp_usbpdev_s *device, u16 interface_number,
    const struct hid_report_descriptor *descriptor, uint8_t led_states);
/* Log descriptor metadata only; never dump input reports or keystrokes. */
void usbhid_print_hid_desc(struct usb_hid_descriptor *hid_descriptor);
#endif
#endif
