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

#ifndef SECOS_LOADER_CFG_READER_H
#define SECOS_LOADER_CFG_READER_H

#include <dev/usb.h>

#ifndef __ASSEMBLY__

#define TYPE_NAME_MAX_LEN	(48)
typedef struct 
{
    char type[TYPE_NAME_MAX_LEN + 1]; // One additional char for the ending '\0'

    uint32_t disp_width;
    uint32_t disp_height;
} SL_DISPLAY_INFO;

typedef struct 
{
    bool ps2_enable;
} SL_PS2_INFO;

typedef struct 
{
    bool usb2_enable;
    uint8_t kbd_dev_addr;
    uint8_t kbd_hub_port;
    uint8_t mouse_dev_addr;
    uint8_t mouse_hub_port;
    uint8_t restore_dev_addr;
    uint8_t restore_hub_port;
} SL_USB2_INFO;

// 
typedef struct 
{
    bool usb3_enable;

    uint8_t hid_dev1_addr;
    uint8_t hid_dev1_hub_addr;
    uint8_t hid_dev1_hub_port;
    uint16_t hid_dev1_bus_id;
    
    uint8_t hid_dev2_addr;
    uint8_t hid_dev2_hub_addr;
    uint8_t hid_dev2_hub_port;
    uint16_t hid_dev2_bus_id;

    uint16_t hid_dev1_vendor_id;
    uint16_t hid_dev1_product_id;
    uint16_t hid_dev2_vendor_id;
    uint16_t hid_dev2_product_id;

    char xhci_hc_pci_addr_str[USB_HC_BDF_STR_LENGTH_MAX + 1]; // One additional char for the ending '\0'

#if defined(__WINDOWS__)
    char xhci_hc_vendor_product_str[USB_HC_WINDOWS_VENDOR_PRODUCT_STR_LENGTH_MAX + 1]; // One additional char for the ending '\0'
#endif
} SL_USB3_INFO;

typedef struct 
{
    SL_DISPLAY_INFO     disp_info;
    SL_PS2_INFO         ps2_info;
    SL_USB2_INFO        usb2_info;
    SL_USB3_INFO        usb3_info;

    // Additional configurations deducted from the config file
    bool need_iosm_pcie;
    bool need_iosm_ps2;
    bool need_iosm_usb2;
    bool need_iosm_usb3;
    bool need_iosm_fb;

    /// @brief Number of components to boot. The components comprise SecOS and IOSMs, but not include DesktopMgr. This
    /// is because DesktopMgr is booted outside SOSLoader.
    uint32_t num_components_to_boot;
} SL_CFG;

/// @brief Read and parse SecServices' configuration files. 
/// @param out_cfg 
/// @return Return SL_SUCCESS on success.
///         Return SL_EINVAL if <out_cfg> is invalid.
///         Return SL_EINVAL_CONFIGS on configuration errors, because they must be caused by invalid settings in the 
///         configuration file.
extern SL_STATUS sl_cfg_read(SL_CFG *out_cfg);

#endif // __ASSEMBLY__
#endif // SECOS_LOADER_CFG_READER_H