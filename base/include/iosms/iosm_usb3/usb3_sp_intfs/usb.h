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


#ifndef _USB_H_
#define _USB_H_

#include "usb_openbsd.h"
#include "iosm_usb3_defs.h"
#include <common/intfs.h>

#ifndef __ASSEMBLY__

struct usb_device;
struct usb_endpoint_descriptor;

struct usb_dma
{
    vaddr_t vaddr;
    paddr_t paddr;
    spaddr_t spaddr;
    size_t size;
    size_t alignment; // Must be 2^n. 0 for no alignment.

    // Dirty hack to allocate memory within boundary
    vaddr_t reallocated_mem_vaddr;
    size_t reallocated_mem_size;
    bool reallocated_for_boundary;
};

struct usb_xfer
{
	struct usb_dma dma;
};

/// @brief USB endpoint.
struct usb_pipe 
{
    USBPDEV_ADDR devaddr;
    uint8_t ep_addr;
    uint8_t ep_type;
    uint8_t ep_speed;
    uint16_t ep_max_packet;

    struct usb_endpoint_descriptor* endpoint;
    struct usb_device* device;

    // Architecture-specific info for <usb_pipe>
    // For example, XHCI sets <pipe_priv> to be the correpsonding xhci_pipe
    void* pipe_priv;
};

#define MAX_DATA_INTERFACES_NUM     (USB_PDEV_ENDPOINT_NUM_MAX - 1)

struct usb_hub
{
    struct usb_device* device;

    uint32_t ports_num;
    bool is_root_hub;
};

struct usb_device
{
    /* USB hierarchy information */
    USBPDEV_ADDR devaddr;
    struct usb_hub* parent_hub;
    uint32_t parent_hub_port; // upstream hub port number (not port index), or 0 for invalid.

    /* USB pipes */
    struct usb_pipe* defpipe;
    struct usb_pipe* data_ep_pipes[MAX_DATA_INTERFACES_NUM]; // Maximum of 15 data pipes (exclude the default control pipe)

    /* USB descriptors */
    struct usb_device_descriptor *ddesc;
    struct usb_config_descriptor *config;
    size_t config_size;
    struct usb_interface_descriptor* data_iface[MAX_DATA_INTERFACES_NUM]; // Interfaces containing data endpoints
    uint32_t data_ifaces_num;
    struct usb_endpoint_descriptor* data_eps[MAX_DATA_INTERFACES_NUM]; // Data endpoints (exclude the default control pipe)
    uint32_t data_eps_num; // Number of data endpoints

    /* Device properties */
    // low/full/high speed
    uint8_t speed;

#define USB_SPEED_LOW	1
#define USB_SPEED_FULL	2
#define USB_SPEED_HIGH	3
#define USB_SPEED_SUPER	4

    // Architecture-specific info for <usb_device>
    // For example, XHCI sets <device_priv> to be the correpsonding xhci_usbdev
    void* device_priv;
};

#define usb_is_root_hub(usbdev) (usbdev->parent_hub == NULL)

#endif // __ASSEMBLY__
#endif // _USB_H_
