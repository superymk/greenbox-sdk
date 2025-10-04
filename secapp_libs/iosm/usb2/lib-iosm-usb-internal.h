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

#ifndef LIBUSB2_HEADERS_H
#define LIBUSB2_HEADERS_H

#include <usb_errno.h>

#include <stdio.h>
#include <string.h>
#include <sap_headers_libc.h>
#include <palddk.h>

#include <iosm_common_intfs.h>

// // Include headers of IOSM-USB2
// #include <iosm_usb2_sp_intfs.h>

#include <utils/usbbuf_mm.h>
// #include <ehci/mgr/eehci_mgr.h>
// #include <ehci/mgr/usbtd_reg_mgr.h>
#include <mgr/usbpdev_mgr.h>

#include <iosm_common_intfs.h>
#include <iosm_common_usb_defs.h>
#include <usb_ops.h>

#endif // LIBUSB2_HEADERS_H