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

// #include <loader_sa_intfs.h>

#include <dev_config/usbhid_probe.h>
// #include <sl_headers.h>
#include <app_var.h>

#include <stdio.h>
#include <libusb.h>

static void _print_device_list(struct usbhid_device_list* device_list);

/// @brief Definition of Invalid addr of USBPDEV.
/// The definition must be consistent with <iosm_usb3.c> of IOSM-USB2 and IOSM-USB3
USBPDEV_ADDR USBADDR_INVALID = {
	.dev_usbaddr = 0,
	.hub_port = 0,
	.hub_usbaddr = 0,
	.bus_id = 0,
	.reserved1 = 0,
	.reserved2 = 0
};

/// @brief "bInterfaceClass: Human Interface Device"
#define USBHID_INTF_CLASS               (0x3) 

#define USBHID_INTF_PROTOCOL_KBD        (0x1)
#define USBHID_INTF_PROTOCOL_MOUSE      (0x2)

/// @brief Convert physical port reported by LibUSB to the virtual port used by the GreenBox.
#define libusb_pport_to_vport(pport)    ((pport) - 1)

// #define USBHID_

static SL_STATUS _usbhid_device_has_interface_type(libusb_device* dev, uint8_t intf_type, bool* out_result)
{
    int ret = 0;
    uint32_t i = 0;
    struct libusb_config_descriptor *dev_config = NULL;

    // Get the active config descriptor of the device
    ret = libusb_get_active_config_descriptor(dev, &dev_config);
    if(ret)
        return SL_EINTERNAL;
    if(!dev_config)
        return SL_EINVAL;

    FOREACH_S(i, dev_config->bNumInterfaces, dev_config->bNumInterfaces, 0, 1)
    {
        const struct libusb_interface_descriptor* intf = &dev_config->interface[i].altsetting[0];

        if(intf)
        {
            if((intf->bInterfaceClass == USBHID_INTF_CLASS) && (intf->bInterfaceProtocol == intf_type))
            {
                // Found the interface with the desired type
                *out_result = true;
                return SL_SUCCESS;
            }
        }
    }

    // Not found any interfaces with the desired type
    *out_result = false;
    return SL_SUCCESS;
}

static SL_STATUS _usbhid_all_devices_query_interface_type(uint8_t intf_type, struct usbhid_device_list* out_device_list)
{
    SL_STATUS status = SL_SUCCESS;
    libusb_device **devs;
    libusb_context *ctx = NULL; // libusb session context
    int r; // for results
    ssize_t dev_cnt; // number of usb devices found
    
    r = libusb_init(&ctx); // open session
    if (r < 0) 
        return SL_EINTERNAL;

    // // set the verbosity level of debug messages
    // libusb_set_debug(ctx, 3);

    // get a list of all found USB devices
    dev_cnt = libusb_get_device_list(ctx, &devs);
    if (dev_cnt < 0) 
        return SL_EINTERNAL;

    // Read and check USB devices' interfaces
    {
        uint32_t i = 0;
        uint32_t dev_list_next_idx = 0;

        FOREACH_S(i, dev_cnt, dev_cnt, 0, 1)
        {
            bool dev_has_intf = false;
            libusb_device *dev = devs[i];

            status = _usbhid_device_has_interface_type(dev, intf_type, &dev_has_intf);
            if(SL_FAIL(status))
                return SL_EINTERNAL;

            if(dev_has_intf)
            {
                // The device has an interface with the given type <intf_type>. 
                // Gather device info.
                struct libusb_device_descriptor dev_desc;
                libusb_device_handle *handle = NULL;
                struct usbhid_device* out_dev_info = &out_device_list->devices[dev_list_next_idx];

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
                    //// Fill <out_dev_info->name> with the device's vendor name and product name
                    unsigned char str_vendor[USBHID_NAME_LENGTH_MAX] = {0};
                    unsigned char str_product[USBHID_NAME_LENGTH_MAX] = {0};

                    libusb_get_string_descriptor_ascii(handle, dev_desc.iManufacturer, str_vendor, USBHID_NAME_LENGTH_MAX);
                    libusb_get_string_descriptor_ascii(handle, dev_desc.iProduct, str_product, USBHID_NAME_LENGTH_MAX);
                    str_vendor[USBHID_NAME_LENGTH_MAX - 1] = '\0';
                    str_product[USBHID_NAME_LENGTH_MAX - 1] = '\0';

                    if(!strstr(str_product, str_vendor))
                        // If the product name does not contain the vendor name, then concat both strings
                        snprintf(out_dev_info->name, USBHID_NAME_LENGTH_MAX, "%s %s", str_vendor, str_product);
                    else
                        // Otherwise, output the product name only
                        snprintf(out_dev_info->name, USBHID_NAME_LENGTH_MAX, "%s", str_product);

                    out_dev_info->name[USBHID_NAME_LENGTH_MAX - 1] = '\0';
                }
                libusb_close(handle);

                // Get device address
                out_dev_info->usb_addr.dev_usbaddr = libusb_get_device_address(dev);
                out_dev_info->usb_addr.hub_port = libusb_pport_to_vport(libusb_get_port_number(dev));

                //// Get the address of the device's upstream hub.
                {
                    libusb_device *hub = libusb_get_parent(dev);

                    if(!hub)
                    {
                        printf("Failed to get the hub for the device %02X:%02X!\n", dev_desc.idVendor, dev_desc.idProduct);
                    }
                    else
                    {
                        out_dev_info->usb_addr.hub_usbaddr = libusb_get_device_address(hub);
                    }
                }

                out_dev_info->usb_addr.bus_id = libusb_get_bus_number(dev);
                out_dev_info->usb_addr.reserved1 = 0xFFFF;
	            out_dev_info->usb_addr.reserved2 = 0x7FF;

                dev_list_next_idx++;
            }

            // Output the first <USBHID_DEVICE_LIST_ENTRIES_MAX> device only 
            if(dev_list_next_idx >= USBHID_DEVICE_LIST_ENTRIES_MAX)
                break;
        }

        out_device_list->num_devices = dev_list_next_idx;
    }

    // release the memory allocated by the get device list function
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx); // close session

    // On success
    return SL_SUCCESS;
}




/********* Public functions *********/
SL_STATUS usbhid_list_all_keyboards(struct usbhid_device_list* out_device_list)
{
    // Check: Parameters must be valid
    if(!out_device_list)
        return SL_EINVAL;

    return _usbhid_all_devices_query_interface_type(USBHID_INTF_PROTOCOL_KBD, out_device_list);
}

SL_STATUS usbhid_list_all_mouses(struct usbhid_device_list* out_device_list)
{
    // Check: Parameters must be valid
    if(!out_device_list)
        return SL_EINVAL;

    return _usbhid_all_devices_query_interface_type(USBHID_INTF_PROTOCOL_MOUSE, out_device_list);
}




/********* Debug functions *********/
static void _print_device_list(struct usbhid_device_list* device_list)
{
    uint32_t i = 0;

    printf("Printing USBHID Device list. %u entries\n", device_list->num_devices);
    FOREACH_S(i, device_list->num_devices, USBHID_DEVICE_LIST_ENTRIES_MAX, 0, 1)
    {
        struct usbhid_device* dev_info = &device_list->devices[i];

        if(dev_info)
        {
            printf("\t\t [Bus %u Dev %u] Name:%s. Hub addr:%u, Hub port:%u\n", 
                dev_info->usb_addr.bus_id, dev_info->usb_addr.dev_usbaddr, 
                dev_info->name, 
                dev_info->usb_addr.hub_usbaddr, dev_info->usb_addr.hub_port);
        }
    }
}