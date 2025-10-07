// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header — Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
 */

#ifndef _COMMON_USB_DEFS_H
#define _COMMON_USB_DEFS_H

#define USB_DEFAULT_ADDR        (0)

#define USB_MAXADDR 127
/// @brief We consider 0 is a valid USB addr, because USB address for USB3 devices stored in untrusted OS is just an ID,
/// not the actual USB address used on the bus. Furthermore, the roothub on Windows 10 uses the address 0.
#define IS_VALID_USBADDR(usbaddr)      ((usbaddr >= 0) && (usbaddr <= USB_MAXADDR))


// [TODO] Borrowed from SeaBios
#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER         0x06
#define USB_DT_OTHER_SPEED_CONFIG       0x07

#define USB_CLASS_PER_INTERFACE         0       /* for DeviceClass */
#define USB_CLASS_AUDIO                 1
#define USB_CLASS_COMM                  2
#define USB_CLASS_HID                   3
#define USB_CLASS_PHYSICAL              5
#define USB_CLASS_STILL_IMAGE           6
#define USB_CLASS_PRINTER               7
#define USB_CLASS_MASS_STORAGE          8
#define USB_CLASS_HUB                   9


#ifndef __ASSEMBLY__

/********* Definitions - I/O Separation *********/
#define IO_PID_INVALID 	        0
#define INVALID_SLOT 	        0xFFFFFFFF

#endif // __ASSEMBLY__
#endif //_COMMON_USB_DEFS_H