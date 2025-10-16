// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header - Limited License
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

#ifndef _IOSM_USB3_PUBLIC_COMMON_INTFS_H
#define _IOSM_USB3_PUBLIC_COMMON_INTFS_H


#ifndef __ASSEMBLY__

#ifndef IOSM_USB_USBPDEV_ADDR
#define IOSM_USB_USBPDEV_ADDR 1
typedef union usbpdev_addr
{
    struct {
        unsigned int dev_usbaddr:7,
                    hub_port:7,
                    hub_usbaddr:7,
                    reserved2:11;
        unsigned int bus_id:16,
                    reserved1:16;
    };
    QWORD v;
} USBPDEV_ADDR;

#endif // IOSM_USB_USBPDEV_ADDR

/// @brief Definition of Invalid addr of USBPDEV.
extern USBPDEV_ADDR USBADDR_INVALID;

#endif// __ASSEMBLY__
#endif //_IOSM_USB3_PUBLIC_COMMON_INTFS_H