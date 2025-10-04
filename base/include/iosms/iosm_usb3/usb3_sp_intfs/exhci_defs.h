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