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

#ifndef LIBUSB3_OPS_H
#define LIBUSB3_OPS_H

#ifndef __ASSEMBLY__
// #include <common/intfs.h>   //iosm-usb3/3
#include <iosm_common_intfs.h>
#include <iosm_usb3_sp_intfs.h>

#define USBD_FORCE_SHORT_XFER	0x08	/* force last short packet on write */

struct usb3_pipe
{
    uint8_t ep;
    USBPDEV_ADDR devaddr;
    uint16_t maxpacket;

    void* data_buf; // Buffer for data pipes (not default control pipe)
    size_t data_buf_size;
    uint32_t last_ioc_trb_id; // The last TRB that will trigger an Event TRB, so Lib-IOSM-USB knows which TRB it should
                    // wait on
};


extern SAP_STATUS libusb3_init(spid_t iosm_usb3_spid, size_t usbbuf_4k_pages);

extern void libusb3_fini(void);

// /// @brief Activate the given USB peripheral device <usbpdev_addr>
extern SAP_STATUS libusb3_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot);

/// @brief Deactivate the given USB peripheral device <usbpdev_addr>
extern SAP_STATUS libusb3_deactivate_usbpdev(USBPDEV_ADDR usbpdev_addr);

// /// @brief Activate an eEHCI
// extern SAP_STATUS libusb3_activate_eehci(uint32_t *out_eehci_slot);

// /// @brief Deactivate the given eEHCI
// extern SAP_STATUS libusb3_deactivate_eehci(uint32_t eehci_slot);


#endif // __ASSEMBLY__
#endif // LIBUSB3_OPS_H