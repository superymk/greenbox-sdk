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

#include <sap_headers_libc.h>
#include <palddk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib-iosm-usb.h>

#include <drv/hid/input_events.h>
#include "usb-mouse.h"
#include "debug.h"
#include "usbhid.h"

extern void libdrv_hid_mouse_event_enqueue(struct mouse_event *event);
extern void libdrv_hid_call_mouse_listeners(struct mouse_event *event);

#define MAX_MOUSE_INTR_PIPES (8)

static struct sp_usbpdev_pipe *_mouse_intr_pipes[MAX_MOUSE_INTR_PIPES] = {0};
static char *mouse_keydata = NULL;
// static bool mouse_isr_enable = false;

static bool pressed_left = false;
static bool pressed_right = false;
static bool pressed_middle = false;

/********* Private Functions *********/
#define MOUSE_KEYDATA_SIZE (8)

static void _pipe_infos_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        _mouse_intr_pipes[i] = NULL;
    }
}

static int _pipe_infos_find(struct sp_usbpdev_pipe *pipe)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        if (_mouse_intr_pipes[i] == pipe)
            return i;
    }

    return -1;
}

// Find the first endpoint of a given type in an interface description.
struct usb_endpoint_descriptor *_mouse_find_ep(
    struct sp_usbpdev_s *usbdev, int type, int dir, struct usb_interface_descriptor *iface)
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
                ((ep->epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type) &&
                ((ep->iface) == iface))
                return ep->epdesc;
        }
    }

    return NULL;
}

#define BYTE1_Y_OVERFLOW (1 << 7)
#define BYTE1_X_OVERFLOW (1 << 6)
#define BYTE1_Y_SIGN_BIT (1 << 5)
#define BYTE1_X_SIGN_BIT (1 << 4)
#define BYTE1_MIDDLE_BTN (1 << 2)
#define BYTE1_RIGHT_BTN (1 << 1)
#define BYTE1_LEFT_BTN (1 << 0)

#define BYTE4_WHEEL_NO_MOVE 0
#define BYTE4_WHEEL_VERTICAL_UP 1
#define BYTE4_WHEEL_VERTICAL_DOWN 0xF
#define BYTE4_WHEEL_HORNIZONTAL_RIGHT 2
#define BYTE4_WHEEL_HORNIZONTAL_LEFT 0xE

static SAP_STATUS _mouse_read_nonblock(struct sp_usbpdev_pipe *pipe, struct mouse_event *out_1st_event)
{
    SAP_STATUS status = SUCCESS;
    struct mouse_event mouse_event = {.action = LIBDRV_HID_MOUSE_NO_EVENT};
    char byte_1, byte_2, byte_3, byte_4;
    // bool y_sign = false, x_sign = false
    bool wheel = false;

    // Sanity checks
    if (!pipe)
    {
        *out_1st_event = mouse_event;
        return EINVAL;
    }

    for (;;)
    {
        status = iosm_usb_poll_intr(pipe, mouse_keydata, MOUSE_KEYDATA_SIZE);
        if (PASS(status))
            // Received key press/release
            break;
        else if (status == E_USBTD_IS_ACTIVE)
        {
            status = SUCCESS;
            mouse_event.action = LIBDRV_HID_MOUSE_NO_EVENT;
            goto out;
        }
        else
        {
            SAP_ERR("QTD error! status:%lu\n", status);
            status = EINTERNAL;
            mouse_event.action = LIBDRV_HID_MOUSE_NO_EVENT;
            goto out;
        }
    }

    // Step 1. Read the mouse input raw data
    // The following mouse message formats follows
    // http://wiki.osdev.org/Mouse_Input
    byte_1 = mouse_keydata[0];
    byte_2 = mouse_keydata[1];
    byte_3 = mouse_keydata[2];
    byte_4 = mouse_keydata[3];

    // Step 2. Handle Byte_1 first
    if ((byte_1 & BYTE1_Y_OVERFLOW) || (byte_1 & BYTE1_X_OVERFLOW))
        goto out; // discard the packet if overflow bit sets

    // y_sign = byte_1 & BYTE1_Y_SIGN_BIT;
    // x_sign = byte_1 & BYTE1_X_SIGN_BIT;

    // Byte_1 - Button
    // [TODO][Issue 129] usb-mouse.c cannot handle button release correctly when multiple buttons were pressed.
    if (byte_1 & BYTE1_MIDDLE_BTN)
    {
        mouse_event.button |= LIBDRV_HID_BTN_MIDDLE;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_PRESSED;
        pressed_middle = true;
    }
    else if (pressed_middle)
    {
        // we must released the middle
        mouse_event.button |= LIBDRV_HID_BTN_MIDDLE;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_RELEASED;
        pressed_middle = false;
    }
    if (byte_1 & BYTE1_RIGHT_BTN)
    {
        mouse_event.button |= LIBDRV_HID_BTN_RIGHT;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_PRESSED;
        pressed_right = true;
    }
    else if (pressed_right)
    {
        // we must released the right
        mouse_event.button |= LIBDRV_HID_BTN_RIGHT;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_RELEASED;
        pressed_right = false;
    }
    if (byte_1 & BYTE1_LEFT_BTN)
    {
        mouse_event.button |= LIBDRV_HID_BTN_LEFT;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_PRESSED;
        pressed_left = true;
    }
    else if (pressed_left)
    {
        // we must released the right
        mouse_event.button |= LIBDRV_HID_BTN_LEFT;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_RELEASED;
        pressed_left = false;
    }

    // Step 3. Handle byte_2 and byte_3
    // The description in http://wiki.osdev.org/Mouse_Input is wrong
    mouse_event.delta_x = (signed char)byte_2;
    mouse_event.delta_y = (signed char)byte_3;
    if (byte_2 || byte_3)
        mouse_event.action = LIBDRV_HID_MOUSE_MOVED;

    // Step 4. Handle byte_4
    if ((byte_4 & 0xF) == BYTE4_WHEEL_VERTICAL_UP)
    {
        mouse_event.wheel = LIBDRV_HID_WHEEL_UP;
        wheel = true;
    }
    else if ((byte_4 & 0xF) == BYTE4_WHEEL_VERTICAL_DOWN)
    {
        mouse_event.wheel = LIBDRV_HID_WHEEL_DOWN;
        wheel = true;
    }
    else if ((byte_4 & 0xF) == BYTE4_WHEEL_HORNIZONTAL_RIGHT)
    {
        mouse_event.wheel = LIBDRV_HID_WHEEL_RIGHT;
        wheel = true;
    }
    else if ((byte_4 & 0xF) == BYTE4_WHEEL_HORNIZONTAL_LEFT)
    {
        mouse_event.wheel = LIBDRV_HID_WHEEL_LEFT;
        wheel = true;
    }

    if (wheel)
    {
        mouse_event.button |= LIBDRV_HID_BTN_WHEEL;
        mouse_event.action = LIBDRV_HID_MOUSE_BTN_PRESSED;
    }

    // On success
    status = SUCCESS;

out:
    *out_1st_event = mouse_event;
    return status;
}

static void _usb_mouse_free_intrpipe(struct sp_usbpdev_pipe *pipe)
{
    SAP_STATUS status = SUCCESS;

    if (pipe)
    {
        int idx = _pipe_infos_find(pipe);

        if (idx != -1)
        {
            status = iosm_usb_usbpdev_free_pipe(pipe);
            if (FAIL(status))
                SAP_ERR("Free USB Interrupt Pipe Error! status:%lu\n", status);

            _mouse_intr_pipes[idx] = NULL;
        }
    }
}

/********* Public functions *********/
SAP_STATUS usb_mouse_mgr_init(void)
{
    _pipe_infos_clean();

    // Allocate mouse_keydata
    if (!mouse_keydata)
    {
        mouse_keydata = (char *)malloc(MOUSE_KEYDATA_SIZE);
        if (!mouse_keydata)
        {
            SAP_ERR("Allocate USB mouse data error!\n");
            return ENOMEM;
        }
    }

    // On success
    return SUCCESS;
}

void usb_mouse_mgr_fini(void)
{
    if (mouse_keydata)
    {
        free(mouse_keydata);
        mouse_keydata = NULL;
    }

    _pipe_infos_clean();
}

struct sp_usbpdev_pipe *usb_mouse_init(struct sp_usbpdev_s *usbmouse_dev, struct usb_interface_descriptor *iface)
{
    SAP_STATUS status = SUCCESS;
    struct sp_usbpdev_pipe *_usbmouse_intr_pipe = NULL;

    // Sanity checks
    if (!usbmouse_dev || !iface)
    {
        SAP_ERR("<usb_mouse_init> usbmouse_dev has not been initialized yet!\n");
        return NULL;
    }

    // Init the interrupt pipe
    struct usb_endpoint_descriptor *epdesc = _mouse_find_ep(
        usbmouse_dev, USB_ENDPOINT_XFER_INT, UE_DIR_IN, iface);
    if (!epdesc)
    {
        SAP_ERR("epdesc is NULL!\n");
        goto get_ep_desc_err;
    }

    if (UGETW(epdesc->wMaxPacketSize) < 3)
    {
        SAP_ERR("Unsupported wMaxPacketSize (%u) of the mouse!\n", UGETW(epdesc->wMaxPacketSize));
        goto get_ep_desc_err;
    }

    SAP_INFO("<usb_mouse_init>: Alloc Mouse Interrupt Pipe\n");
    _usbmouse_intr_pipe = iosm_usb_usbpdev_alloc_intr_pipe(usbmouse_dev, epdesc);
    if (!_usbmouse_intr_pipe)
    {
        SAP_ERR("Allocate USB Mouse interrupt pipe error!\n");
        goto alloc_intr_pipe_err;
    }
    SAP_INFO("Finish allocating USB Mouse interrupt pipe\n");

    if (usbmouse_dev->bus_type == USB3)
    {
        // [TODO] Use BOOT protocol for now. We should switch to REPORT protocol
        if (iface->bInterfaceSubClass == 1)
        {
            // The device supports BOOT protocol, see "Detecting HID devices" on https://wiki.osdev.org/USB_Human_Interface_Devices
            status = set_protocol(usbmouse_dev, 0, iface->bInterfaceNumber);
            if (FAIL(status))
            {
                SAP_ERR("USB Mouse init HID error: failed to set protocol! status:%lu\n", status);
                goto usb_hid_err;
            }

            status = set_idle(usbmouse_dev, 0);
            if (FAIL(status))
            {
                // [TODO][Quirk] Some mouses (e.g., Logitech G502) cannot handle <set_idle> here. So this function
                // continues on error.
                SAP_ERR("USB Mouse init HID error: failed to set idle! status:%lu\n", status);
                // goto usb_hid_err;
            }
        }
        else
        {
            SAP_ERR("USB Mouse init HID error: Device (interface:%u) does not support BOOT protocol!\n", iface->bInterfaceNumber);
            goto usb_hid_err;
        }
    }

    // // Enable ISR
    // mouse_isr_enable = true;

    // Record the interrupt pipe
    {
        int idx = _pipe_infos_find(NULL);

        if (idx == -1)
            goto record_pipe_err;

        _mouse_intr_pipes[idx] = _usbmouse_intr_pipe;
    }

    // On success
    return _usbmouse_intr_pipe;

record_pipe_err:
usb_hid_err:
    if (_usbmouse_intr_pipe)
    {
        _usb_mouse_free_intrpipe(_usbmouse_intr_pipe);
        _usbmouse_intr_pipe = NULL;
    }
alloc_intr_pipe_err:
get_ep_desc_err:
    return NULL;
}

void usb_mouse_fini(struct sp_usbpdev_pipe *pipe)
{
    // // Disable ISR
    // mouse_isr_enable = false;

    // Free kbd USB intr pipe
    _usb_mouse_free_intrpipe(pipe);

    SAP_INFO("Finalized USB Mouse Specific USB pipes\n");
}

SAP_STATUS usb_mouse_read_nonblock(struct mouse_event *out_1st_event)
{
    SAP_STATUS status = SUCCESS;
    uint32_t i = 0;

    if (!out_1st_event)
        return EINVAL;

    FOREACH_S(i, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        struct sp_usbpdev_pipe *pipe = _mouse_intr_pipes[i];
        if (pipe)
        {
            status = _mouse_read_nonblock(pipe, out_1st_event);
            if (FAIL(status))
                return status;

            if (out_1st_event->action != LIBDRV_HID_MOUSE_NO_EVENT)
            {
                // A mouse triggers an event
                return SUCCESS;
            }
        }
    }

    // No mouse triggers an event, just return SUCCESS.
    return SUCCESS;
}

//! \brief Read mouse event due to periodically pull and/or mouse interrupts
SAP_STATUS usb_mouse_handler(void)
{
    SAP_STATUS status = SUCCESS;
    // struct mouse_event event;
    struct mouse_event *event = NULL;

    event = smalloc(sizeof(struct mouse_event));
    if (!event)
    {
        SAP_ERR("No memory to queue the new USB2 mouse event!\n");
        status = ENOMEM;
        goto err;
    }

    status = usb_mouse_read_nonblock(event);
    if (FAIL(status))
    {
        goto err;
    }

    if (event->action != LIBDRV_HID_MOUSE_NO_EVENT)
    {
        print_mouse_event("USB2", event);

        // Call all listeners
        libdrv_hid_call_mouse_listeners(event);
    }

    // On success
    sfree(event, sizeof(struct mouse_event));
    return SUCCESS;

err:
    if (event)
        sfree(event, sizeof(struct mouse_event));
    return status;
}