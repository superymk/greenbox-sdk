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

#if defined(__LINUX__)

#include <dev/usb.h>

#include <appddk.h>
#include <sl_headers.h>
#include <libusb.h>

#include <string.h>

#include "../hash/hash.h"

SL_STATUS usb_hc_get_pci_address(uint32_t usb_bus_id, char* out_pci_addr_str)
{
    libusb_device **devs;
    libusb_context *ctx = NULL; // libusb session context
    int r; // for results
    ssize_t dev_cnt; // number of usb devices found

    if(!out_pci_addr_str)
        return SL_EINVAL;
    
    r = libusb_init(&ctx); // open session
    if (r < 0) 
        return SL_EINTERNAL;

    // get a list of all found USB devices
    dev_cnt = libusb_get_device_list(ctx, &devs);
    if (dev_cnt < 0) 
        return SL_EINTERNAL;

    // Get the PCI bdf of the host controller
    {
        uint32_t i = 0;

        FOREACH_S(i, dev_cnt, dev_cnt, 0, 1)
        {
            libusb_device *dev = devs[i];
            struct libusb_device_descriptor dev_desc;
            libusb_device_handle *handle = NULL;

            // Get the device name string
            r = libusb_get_device_descriptor(dev, &dev_desc);
            if (r < 0) 
            {
                // Error: USB device handle not received
                return SL_EINTERNAL;
            }
            
            libusb_open(dev, &handle);

            if (handle != NULL)
            {
                uint32_t dev_bus_id = libusb_get_bus_number(dev);
                uint32_t dev_usbaddr = libusb_get_device_address(dev);

                if((dev_bus_id == usb_bus_id) && (dev_usbaddr == USB_HC_ROOTHUB_USB_ADDRESS))
                {
                    // Found the root hub for the USB bus
                    libusb_get_string_descriptor_ascii(handle, dev_desc.iSerialNumber, (unsigned char*)out_pci_addr_str, USB_HC_BDF_STR_LENGTH_MAX);
                    out_pci_addr_str[USB_HC_BDF_STR_LENGTH_MAX - 1] = '\0';

                    // On success
                    return SL_SUCCESS;
                }
            }
            
            libusb_close(handle);
        }
    }

    // Not found the root hub for the bus
    return SL_EINVAL;
}

SL_STATUS usb_hc_reset(const char* pci_addr_str)
{
    SL_STATUS status = SL_SUCCESS;
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* exe_filename = "usb_hc_reset.sh";
    const union sha_digest _file_linux_usb_hc_reset_sh_sha256 = {
        .sha2_256_digest = {
            0xfc, 0x83, 0xfe, 0x80, 0xf0, 0xe5, 0x60, 0x9a,
            0x5a, 0x9a, 0xf7, 0x6e, 0xde, 0x3d, 0x1b, 0x78,
            0x38, 0x5e, 0x94, 0xdb, 0xfe, 0xef, 0xaf, 0x79,
            0x45, 0x2f, 0xa7, 0x22, 0xf7, 0x01, 0x33, 0x6f
        }
    };
    int ret = 0;

    if(!pci_addr_str)
        return SL_EINVAL;
    
    // Reset the USB host controller 
    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);
    ret = os_execute_program_wait_exit(false, exe_filepath, &_file_linux_usb_hc_reset_sh_sha256, exe_filename, pci_addr_str, NULL);
    if(ret)
    {
        SL_ERR("Failed to reset the USB host controller %s!\n", pci_addr_str);
        status = SL_EINTERNAL;
    }
    else
    {
        SL_INFO("USB host controller is reset\n");
        status = SL_SUCCESS;
    }

    // Return status
    return status;
}

SL_STATUS usb_hc_disable(const char* pci_addr_str)
{
    SL_STATUS status = SL_SUCCESS;
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* exe_filename = "usb_hc_disable.sh";
    const union sha_digest _file_linux_usb_hc_disable_sh_sha256 = {
        .sha2_256_digest = {
            0x97, 0x27, 0xed, 0xd1, 0x68, 0xcb, 0x82, 0x72,
            0xcc, 0xb0, 0x5d, 0x19, 0x50, 0xaa, 0xec, 0x9c,
            0x22, 0x99, 0xaa, 0x0f, 0xce, 0x46, 0x71, 0x46,
            0xca, 0xec, 0xa7, 0x8b, 0x47, 0x0a, 0xb6, 0x95
        }
    };
    int ret = 0;

    if(!pci_addr_str)
        return SL_EINVAL;
    
    // Reset the USB host controller 
    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);
    ret = os_execute_program_wait_exit(false, exe_filepath, &_file_linux_usb_hc_disable_sh_sha256, exe_filename, pci_addr_str, NULL);
    if(ret)
    {
        SL_ERR("Failed to disable the USB host controller %s!\n", pci_addr_str);
        status = SL_EINTERNAL;
    }
    else
    {
        SL_INFO("USB host controller is disabled\n");
        status = SL_SUCCESS;
    }

    // Return status
    return status;
}


#endif // __LINUX__