// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBUSB2_OPS_H
#define LIBUSB2_OPS_H

#ifndef __ASSEMBLY__
// #include <iosm_usb2_sp_intfs.h>
// #include <common/intfs.h>   //iosm-usb2/3
#include <iosm_common_intfs.h>


extern SAP_STATUS libusb2_init(spid_t iosm_usb2_spid, size_t usbbuf_4k_pages);

extern void libusb2_fini(void);

// /// @brief Activate the given USB peripheral device <usbpdev_addr>
extern SAP_STATUS libusb2_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot);

/// @brief Deactivate the given USB peripheral device <usbpdev_addr>
extern SAP_STATUS libusb2_deactivate_usbpdev(USBPDEV_ADDR usbpdev_addr);

/// @brief Activate an eEHCI
extern SAP_STATUS libusb2_activate_eehci(uint32_t *out_eehci_slot);

/// @brief Deactivate the given eEHCI
extern SAP_STATUS libusb2_deactivate_eehci(uint32_t eehci_slot);

extern struct usb_new_pipe* libusb2_usbpdev_alloc_intr_pipe(
	struct sp_usbpdev_s* usbdev,
    struct usb_endpoint_descriptor* epdesc
);

/// @brief Free a given pipe
extern SAP_STATUS libusb2_usbpdev_free_pipe(struct usb_new_pipe* pipe);


#endif // __ASSEMBLY__
#endif // LIBUSB2_OPS_H