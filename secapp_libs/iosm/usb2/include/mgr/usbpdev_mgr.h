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

#ifndef LIBUSB2_USBPDEV_MGR_H
#define LIBUSB2_USBPDEV_MGR_H

#define USBPDEV_SLOTS           (16)



#ifndef __ASSEMBLY__

extern void usbpdev_mgr_init(void);
extern void usbpdev_mgr_fini(void);

extern SAP_STATUS usbpdev_register(uint32_t slot, USBPDEV_ADDR addr);
extern SAP_STATUS usbpdev_unregister(uint32_t slot);

/// @brief Return the slot that stores <addr>. Return -1 if not found.
extern uint32_t usbpdev_get_slot(USBPDEV_ADDR addr);

#endif // __ASSEMBLY__
#endif // LIBUSB2_USBPDEV_MGR_H