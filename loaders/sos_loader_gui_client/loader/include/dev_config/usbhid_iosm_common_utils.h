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

#ifndef IOSM_USB_COMMON_UTILS_H
#define IOSM_USB_COMMON_UTILS_H

#ifndef __ASSEMBLY__

#define WORD_TO_UINT32(x)       ((uint32_t)(x))



extern bool usbpdev_addr_equal(USBPDEV_ADDR* addr1, USBPDEV_ADDR* addr2);
extern void usbpdev_addr_copy(USBPDEV_ADDR* to, USBPDEV_ADDR* from);

#endif // __ASSEMBLY__
#endif // IOSM_USB_COMMON_UTILS_H