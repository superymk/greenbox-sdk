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

#ifndef LOADER_DEVICE_CONFIG_USB_H
#define LOADER_DEVICE_CONFIG_USB_H

#include <app_types.h>
#include <sl_status.h>
#include <dev_config/usbhid_iosm_common_intfs.h>
#include <dev_config/usbhid_iosm_common_utils.h>

#define USBHID_DEVICE_LIST_ENTRIES_MAX  (4)
#define USBHID_NAME_LENGTH_MAX          (64)

#if defined(__WINDOWS__)
    /// @brief In Windows, the USB address of the USB HC root hub is 0.
    #define USB_HC_ROOTHUB_USB_ADDRESS  (0)
#else
    /// @brief In Linux, the USB address of the USB HC root hub is 1.
    #define USB_HC_ROOTHUB_USB_ADDRESS  (1)
#endif // __WINDOWS__

#ifndef __ASSEMBLY__

struct usbhid_device
{
    USBPDEV_ADDR usb_addr;

    unsigned char name[USBHID_NAME_LENGTH_MAX];
};

/// @brief List of USB devices
struct usbhid_device_list
{
    struct usbhid_device devices[USBHID_DEVICE_LIST_ENTRIES_MAX];

    uint32_t num_devices;
};

extern SL_STATUS usbhid_list_all_keyboards(struct usbhid_device_list* out_device_list);

extern SL_STATUS usbhid_list_all_mouses(struct usbhid_device_list* out_device_list);

#endif // __ASSEMBLY__
#endif // LOADER_DEVICE_CONFIG_USB_H