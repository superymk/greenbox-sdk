// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIB_IOSM_INTERNAL_USB_H
#define LIB_IOSM_INTERNAL_USB_H

#include <palddk.h>
#include <iosm_usb3_sp_intfs.h>
#include <usb_ops.h>

#ifndef __ASSEMBLY__

/// @brief Send a message to the default control pipe of a device.
/// @param usbdev 
/// @param req 
/// @param data 
/// @param timeout_ms 
/// @param enable_debug 
/// @return 
extern SAP_STATUS usb_send_default_control(struct sp_usbpdev_s* usbdev, const struct usb_device_request *req, void *data, uint32_t timeout_ms, bool enable_debug);

/// @brief 
/// @param p 
/// @param dir 
/// @param data 
/// @param datasize 
/// @param timeout_ms 
/// @return 
extern SAP_STATUS usb_ehci_send_bulk(struct usb_new_pipe* p, int dir, void *data, size_t datasize, uint32_t timeout_ms);

/// @brief Issue USB bulk transfers. The transfer must be described in one qTD only.
/// @param p 
/// @param dir 
/// @param data 
/// @param datasize 
/// @param timeout_ms 
/// @return 
extern SAP_STATUS usb_ehci_send_bulk_single_qtd(struct usb_new_pipe* p, int dir, void *data, size_t datasize, uint32_t timeout_ms);

#endif // __ASSEMBLY__
#endif // LIB_IOSM_INTERNAL_USB_H
