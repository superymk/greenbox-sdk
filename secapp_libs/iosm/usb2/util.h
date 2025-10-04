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

#define CPU_YIELD_USEC      (200)

#define msleep(count) usleep(count*1000)
#define usleep(count) usleep(count)
#define yield() usleep(CPU_YIELD_USEC)

/// @brief Sample the current timer value.
/// @param  
/// @return 
extern uint64_t timer_read(void);

extern bool usbpdev_addr_equal(USBPDEV_ADDR* addr1, USBPDEV_ADDR* addr2);

extern void usbpdev_addr_copy(USBPDEV_ADDR* to, USBPDEV_ADDR* from);

#endif // __ASSEMBLY__
#endif // LIB_IOSM_INTERNAL_USB_H