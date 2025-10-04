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

// #include "lib-usb2.h"
#include "usbhid.h"

#include "usb-kbd.h"
#include "usb-mouse.h"

#define MAX_USBHID_DEV_NUM  (8)
typedef enum 
{
	USBHID_INVALID = 0,
	USBHID_KBD = 1,
	USBHID_MOUSE = 12,
} USB_HID_TYPE;

struct usbhid_dev_info
{
    struct sp_usbpdev_s* dev;

    USB_HID_TYPE type;
    struct sp_usbpdev_pipe* pipe;
};

/// @brief Each device <dev> can have one or more entries in <_dev_info>
static struct usbhid_dev_info _dev_info[MAX_USBHID_DEV_NUM] = {0};

/********* Private Functions *********/
static void _dev_infos_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_USBHID_DEV_NUM, MAX_USBHID_DEV_NUM, 0, 1)
    {
        _dev_info[i].dev = NULL;
        _dev_info[i].type = USBHID_INVALID;
        _dev_info[i].pipe = NULL;
    }
}

static int _pipe_infos_find_empty(void)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_USBHID_DEV_NUM, MAX_USBHID_DEV_NUM, 0, 1)
    {
        if(_dev_info[i].dev == NULL)
			return i;
    }

	return -1;
}

/// @brief Init Lib-IOSM-USB
static SAP_STATUS _iosm_usb_client_init(void)
{
    SAP_STATUS status = SUCCESS;
    spid_t iosm_usb2_spid = INVALID_SP_ID;
    spid_t iosm_usb3_spid = INVALID_SP_ID;

    // Get the SecProcess ID of IOSM-USB2 and IOSM-USB3
    iosm_usb2_spid = sos_sys_sp_query_id(SP_SERVICE, "IOSM-USB2", true);
    iosm_usb3_spid = sos_sys_sp_query_id(SP_SERVICE, "IOSM-USB3", true);

    // Init Lib-IOSM-USB
	status = iosm_usb_init(iosm_usb2_spid, SP_USBDRV_USBBUF_4K_PAGES, iosm_usb3_spid, SP_USBDRV_USBBUF_4K_PAGES);
	if(FAIL(status))
	{
		SAP_ERR("Init Lib-IOSM-USB error! status:%lu\n", status);
		return EINTERNAL;
	}

    return SUCCESS;
}

/// @brief Finalize Lib-IOSM-USB
/// @param  
static void _iosm_usb_client_fini(void)
{
    // Fini Lib-USB2
    iosm_usb_fini();
}




/********* Public Functions *********/
#define HID_REQ_SET_IDLE                0x0A
#define HID_REQ_SET_PROTOCOL            0x0B

SAP_STATUS usbhid_mgr_init(void)
{
    SAP_STATUS status = SUCCESS;

    _dev_infos_clean();

    status = usb_kbd_mgr_init();
    if(FAIL(status))
        return status;

    status = usb_mouse_mgr_init();
    if(FAIL(status))
        return status;

    status = _iosm_usb_client_init();
    if(FAIL(status))
        return status;

    // On success
    return SUCCESS;
}

void usbhid_mgr_fini(void)
{
    _iosm_usb_client_fini();

	usb_kbd_mgr_fini();
    usb_mouse_mgr_fini();

    _dev_infos_clean();
}

// Send USB HID protocol message.
SAP_STATUS set_protocol(struct sp_usbpdev_s* dev, u16 val, u16 inferface)
{
    SAP_STATUS status = SUCCESS;
    struct usb_device_request* req = NULL;

    req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));
    if(!req)
        return ENOMEM;

    req->bmRequestType = UT_WRITE_CLASS_INTERFACE;
    req->bRequest = HID_REQ_SET_PROTOCOL;
    USETW(req->wValue, val);
    USETW(req->wIndex, inferface);
    USETW(req->wLength, 0);

    status = iosm_usb_send_default_control(dev, req, NULL, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
    usbbuf_dealloc(USBBUF_MEM_DMA, req);
    if(FAIL(status))
        return EINTERNAL;

    // On success
    return SUCCESS;
}

// Send USB HID SetIdle request.
SAP_STATUS set_idle(struct sp_usbpdev_s* dev, int ms)
{
    SAP_STATUS status = SUCCESS;
    struct usb_device_request* req = NULL;

    req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));
    if(!req)
        return ENOMEM;

    req->bmRequestType = UT_WRITE_CLASS_INTERFACE;
    req->bRequest = HID_REQ_SET_IDLE;
    USETW(req->wValue, (ms/4)<<8);
    USETW(req->wIndex, 0);
    USETW(req->wLength, 0);

    status = iosm_usb_send_default_control(dev, req, NULL, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
    usbbuf_dealloc(USBBUF_MEM_DMA, req);
    if(FAIL(status))
        return EINTERNAL;

    // On success
    return SUCCESS;
}

static SAP_STATUS _dev_init_one_interface(struct sp_usbpdev_s* dev, uint32_t iface_idx)
{
    struct usb_interface_descriptor *iface = NULL;
    struct sp_usbpdev_pipe* pipe = NULL;
    USB_HID_TYPE hid_type = USBHID_INVALID;
    int empty_slot = 0;

    // Check: Parameters must be valid
    if(!dev->data_iface[iface_idx])
        return EINVAL;

    iface = dev->data_iface[iface_idx];

    // Iterate USB interface and init HID driver for each interface
    if(iface->bInterfaceProtocol == USBHID_INTF_PROTOCOL_KBD)
    {
        hid_type = USBHID_KBD;
        pipe = usb_kbd_init(dev, iface);
        if(!pipe)
            return EINTERNAL;
    }
    else if (iface->bInterfaceProtocol == USBHID_INTF_PROTOCOL_MOUSE)
    {
        hid_type = USBHID_MOUSE;
        pipe = usb_mouse_init(dev, iface);
        if(!pipe)
            return EINTERNAL;
    }
    else
    {
        // Not a USB keyboard or mouse
        return E_HID_NOT_KBD_OR_MOUSE;
    }

    // Record the device, hid_type, and the pipe
    empty_slot = _pipe_infos_find_empty();
    if(empty_slot == -1)
    {
        SAP_ERR("USB HID Error: no empty slot for new pipes!\n");
        return ENOMEM;
    }

    _dev_info[empty_slot].dev = dev;
    _dev_info[empty_slot].type = hid_type;
    _dev_info[empty_slot].pipe = pipe;

    // On success
    return SUCCESS;
}

SAP_STATUS usbhid_init_dev(struct sp_usbpdev_s* dev)
{
    SAP_STATUS status = SUCCESS;
    uint32_t i = 0;

    // Check: Parameters must be valid
    if(!dev)
        return EINVAL;

    FOREACH_S(i, dev->data_ifaces_num, USBPDEV_MAX_DATA_INTERFACES_NUM, 0, 1)
    {
        struct usb_interface_descriptor *iface = dev->data_iface[i];

        if(iface)
        {
            status = _dev_init_one_interface(dev, i);
            if(FAIL(status))
            {
                if(status != E_HID_NOT_KBD_OR_MOUSE)
                {
                    SAP_ERR("USB HID Error: Init interface %u error! status:%lu\n", i, status);
                    return status;
                }
            }
        }
    }

    // On success
    return SUCCESS;
}

SAP_STATUS usbhid_fini_dev(struct sp_usbpdev_s* dev)
{
    uint32_t i = 0;

    // Check: Parameters must be valid
    if(!dev)
        return EINVAL;

    FOREACH_S(i, MAX_USBHID_DEV_NUM, MAX_USBHID_DEV_NUM, 0, 1)
    {
        if(_dev_info[i].dev == dev)
        {
            if(_dev_info[i].type == USBHID_KBD)
            {
                usb_kbd_fini(_dev_info[i].pipe);
            }
            else if(_dev_info[i].type == USBHID_MOUSE)
            {
                usb_mouse_fini(_dev_info[i].pipe);
            }

            _dev_info[i].dev = NULL;
            _dev_info[i].type = USBHID_INVALID;
            _dev_info[i].pipe = NULL;
        }
    }

    // On success
    return SUCCESS;
}

SAP_STATUS usb_intr_handler(intr_t intr)
{
    SAP_STATUS status = SUCCESS;

    // For keyboard
    status = usb_kbd_handler();
    if(FAIL(status))
    {
        // Keyboard interrupt error. 
        return status;
    }

    // For mouse
    status = usb_mouse_handler();
    if(FAIL(status))
    {
        // Mouse interrupt error. 
        return status;
    }

    // On success
    return SUCCESS;
}