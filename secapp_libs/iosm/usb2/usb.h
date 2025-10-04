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
