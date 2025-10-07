// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBUSB2_HEADERS_H
#define LIBUSB2_HEADERS_H

#include <usb_errno.h>

#include <stdio.h>
#include <string.h>
#include <sap_headers_libc.h>
#include <palddk.h>

// // Include headers of IOSM-USB2
// #include <iosm_usb2_sp_intfs.h>

// Include headers of IOSM-USB3
#include <iosm_usb3_sp_intfs.h>

#include <utils/usbbuf_mm.h>
// #include <ehci/mgr/eehci_mgr.h>
// #include <ehci/mgr/usbtd_reg_mgr.h>
// #include <mgr/usbpdev_mgr.h>

#include <iosm_common_intfs.h>
#include <iosm_common_usb_defs.h>
#include <usb_ops.h>

#endif // LIBUSB2_HEADERS_H