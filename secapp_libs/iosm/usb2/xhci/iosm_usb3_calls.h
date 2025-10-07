// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOSM_USB3_COMM_IOSM_H
#define IOSM_USB3_COMM_IOSM_H

#include "lib-usb3.h"

#ifndef __ASSEMBLY__

// struct usb3_pipe;
struct sp_trb;


/// @brief Create Inter-SecProcess communication channels required by IOSM-USB3.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_iosm_usb3_init(spid_t in_iosm_usb3_spid);
extern void comm_iosm_usb3_fini(void);



/// @brief Register the caller SecProcess's USB peripheral driver in IOSM-USB3.
///
/// @param do_4k_pages is the number of 4K pages to be used by the driver's USB commands and buffers.
extern SAP_STATUS iosm_usb3_register_usbdrv(size_t buf_4k_pages, uint32_t* out_io_pid, shandle_t* out_h_buf_obj, vaddr_t* out_buf_addr);

extern SAP_STATUS iosm_usb3_unregister_usbdrv(shandle_t h_buf_obj, vaddr_t buf_vaddr, size_t buf_size);

extern SAP_STATUS iosm_usb3_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot);
extern SAP_STATUS iosm_usb3_deactivate_usbpdev(uint32_t usbpdev_slot);
// extern SAP_STATUS __iosm_usb3_activate_eehci(union eEHCI_ID* out_eehci_id, uint32_t* out_eehci_handle, uint32_t* out_eehci_slot);
// extern SAP_STATUS __iosm_usb3_deactivate_eehci(uint32_t eehci_slot);
// extern SAP_STATUS iosm_usb3_reactivate_qtd(uint32_t td_slot);
// extern SAP_STATUS iosm_usb3_ehci_waittick(void);
// extern SAP_STATUS iosm_usb3_query_usbtd_status(uint32_t td_slot, uint32_t* out_td_status);
// extern SAP_STATUS iosm_usb3_attach_qh_intr(
//     uint8_t pipe_eptype, 
//     uint32_t pipe_qh_slot, 
//     int pipe_frameexp, 
//     bool need_circular_qtd, 
//     uint32_t cir_from_qtd_slot, 
//     uint32_t cir_to_qtd_slot, 
//     usbpipe_handle_t* out_h_usbpipe
// );
// extern SAP_STATUS iosm_usb3_deattach_qh(usbpipe_handle_t h_usbpipe, uint8_t pipe_eptype);
// extern SAP_STATUS iosm_usb3_allocate_usbtd(uint32_t td_type, uint32_t* out_td_slot);
// extern SAP_STATUS iosm_usb3_deallocate_usbtd(uint32_t td_slot);
// extern SAP_STATUS iosm_usb3_usbtd_usbmit_and_verify(
//     void* td,
//     size_t td_size, 
//     uint32_t td_slot, 
//     union eEHCI_ID eehci_id, 
//     uint32_t usbpdev_slot, 
//     uint32_t other_params1,
//     uint32_t other_params2,
//     uint32_t other_params3
// );
// extern SAP_STATUS iosm_usb3_eehci_write_usbtd_reg(uint32_t usbtd_reg_id, uint32_t new_v);

extern SAP_STATUS iosm_usb3_submit_trbs(
    struct usb3_pipe* p,
    struct sp_trb* trbs,
    uint32_t num_trbs,
    uint32_t trbs_flags,
    bool iosm_usb3_op_enable_debug
);

extern SAP_STATUS iosm_usb3_query_trb_status(USBPDEV_ADDR devaddr, uint32_t ep, uint32_t trb_id, uint32_t* out_trb_status, bool iosm_usb3_op_enable_debug);
extern SAP_STATUS iosm_usb3_get_usbpdev_speed(USBPDEV_ADDR usbpdev_addr, uint8_t* out_speed);

/// @brief Get the USB device descriptors stored in IOSM-USB3.
/// @param usbpdev_addr 
/// @param out_usbpdev_dev_desc 
/// @param out_usbpdev_config_desc 
/// @return 
extern SAP_STATUS iosm_usb3_usbpdev_get_desc(
    USBPDEV_ADDR usbpdev_addr, 
    struct usb_device_descriptor* out_usbpdev_dev_desc, 
    struct usb_config_descriptor** out_p_usbpdev_config_desc,
    bool iosm_usb3_op_enable_debug
);



/********* Debug functions *********/
extern SAP_STATUS iosm_usb3_debug_print_xhci(void);

#endif // __ASSEMBLY__
#endif // IOSM_USB3_COMM_IOSM_H