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

#ifndef IOSM_USB2_COMM_IOSM_H
#define IOSM_USB2_COMM_IOSM_H



#ifndef __ASSEMBLY__



/// @brief Create Inter-SecProcess communication channels required by IOSM-USB2.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_iosm_usb2_init(spid_t in_iosm_usb2_spid);
extern void comm_iosm_usb2_fini(void);



/// @brief Register the caller SecProcess's USB peripheral driver in IOSM-USB2.
///
/// @param do_4k_pages is the number of 4K pages to be used by the driver's USB commands and buffers.
extern SAP_STATUS iosm_usb2_register_usbdrv(size_t buf_4k_pages, uint32_t* out_io_pid, shandle_t* out_h_buf_obj, vaddr_t* out_buf_addr);

extern SAP_STATUS iosm_usb2_unregister_usbdrv(shandle_t h_buf_obj, vaddr_t buf_vaddr, size_t buf_size);

extern SAP_STATUS iosm_usb2_activate_usbpdev(USBPDEV_ADDR usbpdev_addr, uint32_t* out_usbpdev_slot);
extern SAP_STATUS iosm_usb2_deactivate_usbpdev(uint32_t usbpdev_slot);
extern SAP_STATUS __iosm_usb2_activate_eehci(union eEHCI_ID* out_eehci_id, uint32_t* out_eehci_handle, uint32_t* out_eehci_slot);
extern SAP_STATUS __iosm_usb2_deactivate_eehci(uint32_t eehci_slot);
extern SAP_STATUS iosm_usb2_reactivate_qtd(uint32_t td_slot);
extern SAP_STATUS iosm_usb2_ehci_waittick(void);
extern SAP_STATUS iosm_usb2_query_usbtd_status(uint32_t td_slot, uint32_t* out_td_status);
extern SAP_STATUS iosm_usb2_attach_qh_intr(
    uint8_t pipe_eptype, 
    uint32_t pipe_qh_slot, 
    int pipe_frameexp, 
    bool need_circular_qtd, 
    uint32_t cir_from_qtd_slot, 
    uint32_t cir_to_qtd_slot, 
    usbpipe_handle_t* out_h_usbpipe
);
extern SAP_STATUS iosm_usb2_deattach_qh(usbpipe_handle_t h_usbpipe, uint8_t pipe_eptype);
extern SAP_STATUS iosm_usb2_allocate_usbtd(uint32_t td_type, uint32_t* out_td_slot);
extern SAP_STATUS iosm_usb2_deallocate_usbtd(uint32_t td_slot);
extern SAP_STATUS iosm_usb2_usbtd_usbmit_and_verify(
    void* td,
    size_t td_size, 
    uint32_t td_slot, 
    union eEHCI_ID eehci_id, 
    uint32_t usbpdev_slot, 
    uint32_t other_params1,
    uint32_t other_params2,
    uint32_t other_params3
);
extern SAP_STATUS iosm_usb2_eehci_write_usbtd_reg(uint32_t usbtd_reg_id, uint32_t new_v);

extern SAP_STATUS iosm_usb2_get_usbpdev_speed(USBPDEV_ADDR usbpdev_addr, uint8_t* out_speed);




/********* Debug functions *********/
extern SAP_STATUS iosm_usb2_debug_print_ehci(void);

#endif // __ASSEMBLY__
#endif // IOSM_USB2_COMM_IOSM_H