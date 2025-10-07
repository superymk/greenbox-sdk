// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "usb.h"

#include <lib-iosm-usb.h>

#include "sap_mm.h"
// #include "./ehci/ehci.h"
#include "./xhci/xhci.h"

#include "util.h"

// Send a message on a control pipe using the default control descriptor.
static SAP_STATUS _usb_send_control(
    USB_BUS_TYPE bus_type,
    void *pipe,
    int dir,
    const void *cmd,
    int cmdsize,
    void *data,
    int datasize,
    u16 maxpacket,
    uint32_t timeout_ms,
    bool enable_debug)
{
    SAP_STATUS status = SUCCESS;

    if (bus_type == USB2)
    {
        // status = ehci_send_control((struct usb_new_pipe *)pipe, dir, cmd, cmdsize, data, datasize, maxpacket, timeout_ms);
        USB_ERR("<_usb_send_control>: Unsupported!\n");
        return EINTERNAL;
    }
    else if (bus_type == USB3)
        status = xhci_send_control((struct usb3_pipe *)pipe, dir, cmd, cmdsize, data, datasize, maxpacket, timeout_ms, enable_debug);
    else
    {
        // Invalid USB bus type <bus_type>
        return EINVAL;
    }

    return status;
}

SAP_STATUS libusb2_send_bulk(void *pipe_fl, int dir, void *data, int datasize, uint32_t timeout_ms)
{
    SAP_STATUS status = SUCCESS;

    // status = usb_ehci_send_bulk_single_qtd(pipe_fl, dir, data, datasize, timeout_ms);
    USB_ERR("<libusb2_send_bulk>: Unsupported!\n");
    status = EINTERNAL;

    return status;
}

// Send a message to the default control pipe of a device.
SAP_STATUS usb_send_default_control(struct sp_usbpdev_s *usbdev, const struct usb_device_request *req, void *data, uint32_t timeout_ms, bool enable_debug)
{
    int no_data = 0;
    SAP_STATUS ret = SUCCESS;
    uint16_t maxpacket = 0;
    void *pipe = usbdev->defpipe;

    if (usbdev->bus_type == USB2)
    {
        // maxpacket = ((struct usb_new_pipe *)pipe)->maxpacket;

        // if (!data)
        // {
        //     no_data = 1;
        //     data = usbbuf_alloc(USBBUF_MEM_DMA, 64);
        // }

        USB_ERR("<usb_send_default_control>: Unsupported!\n");
        return EINTERNAL;
    }
    else if (usbdev->bus_type == USB3)
    {
        maxpacket = ((struct usb3_pipe *)pipe)->maxpacket;
    }

    ret = _usb_send_control(usbdev->bus_type, pipe, req->bmRequestType,
                       req, sizeof(*req), data, UGETW(req->wLength), maxpacket, timeout_ms, enable_debug);

    if (usbdev->bus_type == USB2)
    {
        if (no_data)
        {
            usbbuf_dealloc(USBBUF_MEM_DMA, data);
        }
    }

    return ret;
}

// Find the first endpoint of a given type in an interface description.
struct usb_endpoint_descriptor *libusb2_findEndPointDesc(
    struct sp_usbpdev_s *usbdev, int type, int dir)
{
    uint32_t i = 0;

    // Check: Parameters must be valid
    if (!usbdev)
        return NULL;

    FOREACH_S(i, usbdev->data_eps_num, USBPDEV_MAX_DATA_ENDPOINTS_NUM, 0, 1)
    {
        struct usbpdev_endpoint_descriptor *ep = &usbdev->data_eps[i];

        if (ep && ep->iface && ep->epdesc)
        {
            if (((ep->epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir) &&
                ((ep->epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type))
                return ep->epdesc;
        }
    }

    return NULL;
}