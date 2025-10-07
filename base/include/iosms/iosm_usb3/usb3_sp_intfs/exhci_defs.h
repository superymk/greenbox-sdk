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

#ifndef IOSM_USB3_EXHCI_DEFS_H
#define IOSM_USB3_EXHCI_DEFS_H

// #include <common/intfs.h>

// // Same as <XHCI_RING_ITEMS> in usb-xhci.h
// #define NUM_RING_ITEMS  (16)

// // An eXHCI can only support 7 endpoints, or it will be more than 4KB
// #define EXHCI_NUM_ENDPOINT_CONTEXT    (7)

#ifndef __ASSEMBLY__

// transfer request block for rings defined in XHCI
struct ring_trb
{
    uint32_t ptr_low;
    uint32_t ptr_high;
    uint32_t status;
    uint32_t control;
};

// struct event_ring
// {
//     struct ring_trb ring[NUM_RING_ITEMS];
// };

// struct endpoint_context
// {
//     uint32_t ep;

//     // Endpoint ring
//     struct ring_trb ring[NUM_RING_ITEMS];

//     // Event ring
//     struct event_ring evt_ring;
// };

// /// @brief Each ephemeral XHCI is for a USB device
// struct exhci
// {
//     // (Read-only) The USB device supported by this ephemeral XHCI
//     USBPDEV_ADDR usb_addr; 

//     // Device endpoints (include default control endpoint)
//     struct endpoint_context ep_ctxt[EXHCI_NUM_ENDPOINT_CONTEXT];
// };

#endif // __ASSEMBLY__
#endif // IOSM_USB3_EXHCI_DEFS_H