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

#ifndef SOS_LOADER_DEV_USB_H
#define SOS_LOADER_DEV_USB_H

#include <sl_status.h>
#include <app_types.h>

// #include <iosm_pcie_app_intfs.h>

#define USB_HC_BDF_STR_LENGTH_MAX                   (24)

/// @brief e.g., "VEN_8086&DEV_A2AF"
#define USB_HC_WINDOWS_VENDOR_PRODUCT_STR_LENGTH_MAX (20)

#if defined(__WINDOWS__)
    /// @brief In Windows, the USB address of the USB HC root hub is 0.
    #define USB_HC_ROOTHUB_USB_ADDRESS  (0)
#else
    /// @brief In Linux, the USB address of the USB HC root hub is 1.
    #define USB_HC_ROOTHUB_USB_ADDRESS  (1)
#endif // __WINDOWS__

#ifndef __ASSEMBLY__

/// @brief Return the PCI address of the USB host controller. The format is "XXXX:XX:XX.X" ("%04x:%02x:%02x.%u").
/// @param usb_bus_id 
/// @param out_pci_addr_str 
/// @return 
extern SL_STATUS usb_hc_get_pci_address(uint32_t usb_bus_id, char* out_pci_addr_str);

/// @brief Convert OS-specific string of USB host controller's PCI address to PCI BDF struct
/// @return Return the BDF of the device if the input is good. 
///         Return 0:0.0 if the input is malformed. 
extern union pci_bdf os_usb_pci_addr_string_to_bdf(char* usb_hc_pci_bdf);

#if defined(__LINUX__)
    /// @brief Reset the USB host controller at the given PCI address. The format is "XXXX:XX:XX.X" ("%04x:%02x:%02x.%u").
    /// @param pci_addr_str 
    /// @return 
    extern SL_STATUS usb_hc_reset(const char* pci_addr_str);

    /// @brief Disable the USB host controller at the given PCI address. The format is "XXXX:XX:XX.X" ("%04x:%02x:%02x.%u").
    /// @param pci_addr_str 
    /// @return 
    extern SL_STATUS usb_hc_disable(const char* pci_addr_str);
#elif defined(__WINDOWS__)
    extern SL_STATUS usb_hc_get_vendor_product_string(uint32_t usb_bus_id, char* out_usb_hc_vendor_product_str);

    /// @brief Reset the USB host controller with the given vendor and product string.
    /// @param usb_hc_vendor_product_str 
    /// @return 
    extern SL_STATUS usb_hc_reset_by_vendor_product_str(char* usb_hc_vendor_product_str);

    /// @brief Disable the USB host controller with the given vendor and product string.
    /// @param usb_hc_vendor_product_str 
    /// @return 
    extern SL_STATUS usb_hc_disable_by_vendor_product_str(char* usb_hc_vendor_product_str);
#endif // __WINDOWS__

#endif // __ASSEMBLY__
#endif // SOS_LOADER_DEV_USB_H