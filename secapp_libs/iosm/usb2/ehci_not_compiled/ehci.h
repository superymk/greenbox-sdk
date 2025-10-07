// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Zongwei Zhou (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// Main code for handling USB controllers and devices. (from Seabios)
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef _PAL_USB_EHCI
#define _PAL_USB_EHCI

// #include "usb.h"
#include <iosm_usb2_sp_intfs.h>
#include <palddk.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS ehci_send_control(
	struct usb_new_pipe* p, 
	int dir, 
	const void *cmd, 
	size_t cmdsize,
    void* data, 
	size_t datasize, 
	u16 maxpacket,
	uint32_t td_timeout_ms
);
extern SAP_STATUS libusb2_poll_intr(struct usb_new_pipe *p, void *data, size_t data_sz);

extern SAP_STATUS usb2_usbdev_alloc_defpipe(struct sp_usbpdev_s* dev);
extern SAP_STATUS usb2_usbdev_free_defpipe(struct sp_usbpdev_s* dev);

/// @brief Free a given pipe
extern SAP_STATUS libusb2_usbpdev_free_pipe(struct usb_new_pipe* pipe);



// [TODO] Move it to a better place
extern struct pal_utask* pal_get_info(void);





/********* Debug Functions *********/
extern void pal_usb_dump_usbtd(vaddr_t vaddr);

#endif // __ASSEMBLY__
#endif //_PAL_USB_EHCI