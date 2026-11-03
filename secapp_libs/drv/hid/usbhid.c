// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
#include "usbhid.h"
#include "usb-kbd.h"
#include "usb-mouse.h"
#include <stdlib.h>
#include <string.h>

#define HID_CLASS 3
#define HID_BOOT_SUBCLASS 1
#define HID_REPORT_DESCRIPTOR 0x22
#define HID_REQUEST_SET_REPORT 0x09
#define HID_REQUEST_SET_IDLE 0x0a
#define HID_REQUEST_SET_PROTOCOL 0x0b
#define HID_OUTPUT_REPORT_TYPE 2u
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

static bool hid_manager_initialized;

/* Standard BOOT layouts are decoded by the same descriptor parser. BOOT mouse
 * defines three buttons and X/Y; a fourth byte is not a defined wheel. */
static const uint8_t boot_keyboard_descriptor[] = {
    0x05,1,0x09,6,0xa1,1,0x05,7,0x19,0xe0,0x29,0xe7,
    0x15,0,0x25,1,0x75,1,0x95,8,0x81,2,
    0x75,8,0x95,1,0x81,1,
    0x05,8,0x19,1,0x29,5,0x75,1,0x95,5,0x91,2,
    0x75,3,0x95,1,0x91,1,
    0x05,7,0x19,0,0x29,0x65,0x15,0,0x25,0x65,
    0x75,8,0x95,6,0x81,0,0xc0
};
static const uint8_t boot_mouse_descriptor[] = {
    0x05,1,0x09,2,0xa1,1,0x09,1,0xa1,0,
    0x05,9,0x19,1,0x29,3,0x15,0,0x25,1,
    0x75,1,0x95,3,0x81,2,0x75,5,0x95,1,0x81,1,
    0x05,1,0x09,0x30,0x09,0x31,0x15,0x81,0x25,0x7f,
    0x75,8,0x95,2,0x81,6,0xc0,0xc0
};
static unsigned boot_input_type(const struct usb_interface_descriptor *interface_descriptor)
{
    if (interface_descriptor->bInterfaceClass != HID_CLASS ||
        interface_descriptor->bInterfaceSubClass != HID_BOOT_SUBCLASS)
    {
        return 0;
    }

    if (interface_descriptor->bInterfaceProtocol == USBHID_INTF_PROTOCOL_KBD)
    {
        return HID_REPORT_KEYBOARD;
    }

    if (interface_descriptor->bInterfaceProtocol == USBHID_INTF_PROTOCOL_MOUSE)
    {
        return HID_REPORT_MOUSE;
    }

    return 0;
}

/* Find the first interrupt IN endpoint owned by the given interface. */
static struct usbpdev_endpoint_descriptor *find_interrupt_input_endpoint(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor)
{
    uint32_t endpoint_index;

    if (device->data_eps_num > USBPDEV_MAX_DATA_ENDPOINTS_NUM)
    {
        return NULL;
    }

    FOREACH_S(endpoint_index, device->data_eps_num, USBPDEV_MAX_DATA_ENDPOINTS_NUM, 0, 1)
    {
        struct usbpdev_endpoint_descriptor *endpoint = &device->data_eps[endpoint_index];

        if (endpoint->iface == interface_descriptor && endpoint->epdesc &&
            (endpoint->epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == UE_DIR_IN &&
            (endpoint->epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT)
        {
            return endpoint;
        }
    }

    return NULL;
}

SAP_STATUS read_hid_desc_content(struct sp_usbpdev_s *device,
    struct usbpdev_endpoint_descriptor *endpoint, uint8_t *output_descriptor, size_t output_capacity)
{
    SAP_STATUS status;
    struct usb_device_request *control_request;
    uint8_t *descriptor_dma_buffer;
    size_t descriptor_length;

    if (!device || !endpoint || !endpoint->iface || !endpoint->hid_desc || !output_descriptor)
    {
        return EINVAL;
    }

    descriptor_length = endpoint->hid_desc->wClassDescriptorLength;
    if (!descriptor_length || descriptor_length > HID_REPORT_MAX_DESCRIPTOR || descriptor_length > output_capacity ||
        endpoint->hid_desc->bClassDescriptorType != HID_REPORT_DESCRIPTOR)
    {
        return EINVAL;
    }

    control_request = usbbuf_alloc(USBBUF_MEM_DMA, sizeof(*control_request));
    if (!control_request)
    {
        return ENOMEM;
    }

    descriptor_dma_buffer = usbbuf_alloc(USBBUF_MEM_DMA, descriptor_length);
    if (!descriptor_dma_buffer)
    {
        usbbuf_dealloc(USBBUF_MEM_DMA, control_request);
        return ENOMEM;
    }

    memset(control_request, 0, sizeof(*control_request));
    memset(descriptor_dma_buffer, 0, descriptor_length);
    control_request->bmRequestType = UT_READ_INTERFACE;
    control_request->bRequest = UR_GET_DESCRIPTOR;
    USETW(control_request->wValue, HID_REPORT_DESCRIPTOR << 8);
    USETW(control_request->wIndex, endpoint->iface->bInterfaceNumber);
    USETW(control_request->wLength, descriptor_length);

    status = iosm_usb_send_default_control(device, control_request, descriptor_dma_buffer, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
    if (PASS(status))
    {
        memcpy(output_descriptor, descriptor_dma_buffer, descriptor_length);
    }

    usbbuf_dealloc(USBBUF_MEM_DMA, descriptor_dma_buffer);
    usbbuf_dealloc(USBBUF_MEM_DMA, control_request);

    return status;
}

static SAP_STATUS send_class_control_request(struct sp_usbpdev_s *device, uint8_t request_code,
    u16 request_value, u16 interface_number)
{
    struct usb_device_request *control_request;
    SAP_STATUS status;

    if (!device)
    {
        return EINVAL;
    }

    control_request = usbbuf_alloc(USBBUF_MEM_DMA, sizeof(*control_request));
    if (!control_request)
    {
        return ENOMEM;
    }

    memset(control_request, 0, sizeof(*control_request));
    control_request->bmRequestType = UT_WRITE_CLASS_INTERFACE;
    control_request->bRequest = request_code;
    USETW(control_request->wValue, request_value);
    USETW(control_request->wIndex, interface_number);
    USETW(control_request->wLength, 0);

    status = iosm_usb_send_default_control(device, control_request, NULL, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
    usbbuf_dealloc(USBBUF_MEM_DMA, control_request);

    return status;
}

// Send USB HID protocol message.
SAP_STATUS set_protocol(struct sp_usbpdev_s *device, u16 selected_protocol, u16 interface_number)
{
    if (selected_protocol > USBHID_PROTOCOL_REPORT)
    {
        return EINVAL;
    }

    return send_class_control_request(device, HID_REQUEST_SET_PROTOCOL, selected_protocol, interface_number);
}

// Send USB HID SetIdle request.
SAP_STATUS set_idle(struct sp_usbpdev_s *device, int milliseconds, u16 interface_number)
{
    if (milliseconds < 0 || milliseconds > 1020)
    {
        return EINVAL;
    }

    /* SET_IDLE packs duration in 4 ms units into the high byte; ID zero
     * in the low byte applies the duration to every report on the interface. */
    return send_class_control_request(device, HID_REQUEST_SET_IDLE,
        (u16)((milliseconds / 4) << 8), interface_number);
}


SAP_STATUS usbhid_prepare_interface(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor, unsigned requested_input_type,
    struct usbpdev_endpoint_descriptor **output_endpoint,
    struct hid_report_descriptor **output_descriptor, bool *output_boot_protocol)
{
    struct usbpdev_endpoint_descriptor *endpoint;
    struct hid_report_descriptor *descriptor = NULL;
    uint8_t *descriptor_bytes = NULL;
    size_t descriptor_length, packet_capacity;
    unsigned boot_input;
    SAP_STATUS status = EINVAL;
    int parse_result;
    bool boot_protocol = false;
    const char *fallback_reason = NULL;

    if (!output_endpoint || !output_descriptor || !output_boot_protocol)
    {
        return EINVAL;
    }

    *output_endpoint = NULL;
    *output_descriptor = NULL;
    *output_boot_protocol = false;

    if (!device || !interface_descriptor ||
        (requested_input_type && requested_input_type != HID_REPORT_KEYBOARD &&
         requested_input_type != HID_REPORT_MOUSE))
    {
        return EINVAL;
    }

    if (interface_descriptor->bInterfaceClass != HID_CLASS)
    {
        return E_HID_NOT_KBD_OR_MOUSE;
    }

    endpoint = find_interrupt_input_endpoint(device, interface_descriptor);
    if (!endpoint || !endpoint->hid_desc)
    {
        return EINVAL;
    }

    /* Check the raw encoding before the controller sees it: its allocator uses
     * the raw word, while transfer sizing masks it to eleven size bits. This
     * driver does not support additional high-bandwidth transactions. */
    packet_capacity = UGETW(endpoint->epdesc->wMaxPacketSize);
    if (!packet_capacity || packet_capacity > HID_REPORT_MAX_PACKET)
    {
        return EINVAL;
    }

    if (endpoint->hid_desc->bDescriptorType != 0x21 ||
        !endpoint->hid_desc->bNumDescriptors ||
        endpoint->hid_desc->bLength < 6u + 3u * endpoint->hid_desc->bNumDescriptors ||
        endpoint->hid_desc->bClassDescriptorType != HID_REPORT_DESCRIPTOR)
    {
        return EINVAL;
    }

    descriptor_length = endpoint->hid_desc->wClassDescriptorLength;
    if (!descriptor_length)
    {
        return EINVAL;
    }

    descriptor = calloc(1, sizeof(*descriptor));
    if (!descriptor)
    {
        return ENOMEM;
    }

    if (descriptor_length > HID_REPORT_MAX_DESCRIPTOR)
    {
        fallback_reason = "REPORT descriptor exceeds parser capacity";
    }
    else
    {
        descriptor_bytes = malloc(descriptor_length);
        if (!descriptor_bytes)
        {
            status = ENOMEM;
            goto fail;
        }

        status = read_hid_desc_content(device, endpoint, descriptor_bytes, descriptor_length);
        /* The supplied controller returns EIO for STALL, timeout, and other
         * failures. None establishes that REPORT is unsupported. */
        if (FAIL(status))
        {
            goto fail;
        }

        parse_result = hid_report_parse(descriptor_bytes, descriptor_length, descriptor);
        if (parse_result == HID_PARSE_INVALID)
        {
            status = EINVAL;
            goto fail;
        }

        if (parse_result == HID_PARSE_UNSUPPORTED)
        {
            fallback_reason = "REPORT layout unsupported by driver";
        }
        else if (descriptor->max_input_bytes > packet_capacity)
        {
            fallback_reason = "REPORT exceeds interrupt packet capacity";
        }
        else if (descriptor->input_types == (HID_REPORT_KEYBOARD | HID_REPORT_MOUSE))
        {
            fallback_reason = "keyboard and mouse share one REPORT interface";
        }
        else if (!descriptor->input_types ||
                 (requested_input_type && descriptor->input_types != requested_input_type))
        {
            status = E_HID_NOT_KBD_OR_MOUSE;
            goto fail;
        }
    }

    if (fallback_reason)
    {
        boot_input = boot_input_type(interface_descriptor);
        if (!boot_input || (requested_input_type && boot_input != requested_input_type))
        {
            status = E_HID_NOT_KBD_OR_MOUSE;
            goto fail;
        }

        if (boot_input == HID_REPORT_KEYBOARD)
        {
            parse_result = hid_report_parse(boot_keyboard_descriptor, sizeof(boot_keyboard_descriptor), descriptor);
        }
        else
        {
            parse_result = hid_report_parse(boot_mouse_descriptor, sizeof(boot_mouse_descriptor), descriptor);
        }

        if (parse_result != HID_PARSE_OK || descriptor->max_input_bytes > packet_capacity)
        {
            status = EINVAL;
            goto fail;
        }

        status = set_protocol(device, USBHID_PROTOCOL_BOOT, interface_descriptor->bInterfaceNumber);
        if (FAIL(status))
        {
            goto fail;
        }

        boot_protocol = true;
        SAP_INFO("USB HID interface %u: REPORT unsupported: %s\n",
            interface_descriptor->bInterfaceNumber, fallback_reason);
    }
    else if (interface_descriptor->bInterfaceSubClass == HID_BOOT_SUBCLASS)
    {
        /* SET_PROTOCOL is defined only for the BOOT subclass. Other HID
         * interfaces always use their descriptor-defined REPORT format. */
        status = set_protocol(device, USBHID_PROTOCOL_REPORT, interface_descriptor->bInterfaceNumber);
        if (FAIL(status))
        {
            goto fail;
        }
    }

    /* HID Appendix G requires SET_IDLE for keyboards, but it is optional for
     * mice. A mouse may STALL it and halt the shared xHCI control endpoint. */
    if (descriptor->input_types == HID_REPORT_KEYBOARD)
    {
        status = set_idle(device, 0, interface_descriptor->bInterfaceNumber);
        if (FAIL(status))
        {
            SAP_INFO("USB HID interface %u: SET_IDLE unavailable (status:%lu)\n",
                interface_descriptor->bInterfaceNumber, status);
        }
    }

    free(descriptor_bytes);
    *output_endpoint = endpoint;
    *output_descriptor = descriptor;
    *output_boot_protocol = boot_protocol;

    return SUCCESS;

fail:
    free(descriptor_bytes);
    free(descriptor);

    return status;
}

static SAP_STATUS send_output_report(struct sp_usbpdev_s *device, u16 interface_number, const uint8_t *output_report,
    size_t output_report_length, uint8_t report_id)
{
    SAP_STATUS status;
    struct usb_device_request *control_request = usbbuf_alloc(USBBUF_MEM_DMA, sizeof(*control_request));
    uint8_t *dma_buffer;

    if (!control_request)
    {
        return ENOMEM;
    }

    dma_buffer = usbbuf_alloc(USBBUF_MEM_DMA, output_report_length);
    if (!dma_buffer)
    {
        usbbuf_dealloc(USBBUF_MEM_DMA, control_request);
        return ENOMEM;
    }

    memcpy(dma_buffer, output_report, output_report_length);
    memset(control_request, 0, sizeof(*control_request));
    control_request->bmRequestType = UT_WRITE_CLASS_INTERFACE;
    control_request->bRequest = HID_REQUEST_SET_REPORT;
    USETW(control_request->wValue, (HID_OUTPUT_REPORT_TYPE << 8) | report_id);
    USETW(control_request->wIndex, interface_number);
    USETW(control_request->wLength, output_report_length);

    status = iosm_usb_send_default_control(device, control_request, dma_buffer, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
    usbbuf_dealloc(USBBUF_MEM_DMA, dma_buffer);
    usbbuf_dealloc(USBBUF_MEM_DMA, control_request);

    return status;
}

SAP_STATUS usbhid_send_leds(struct sp_usbpdev_s *device, u16 interface_number,
    const struct hid_report_descriptor *descriptor, uint8_t led_states)
{
    uint8_t output_report[HID_REPORT_MAX_PACKET], report_id;
    size_t output_report_length;
    unsigned output_report_index;
    SAP_STATUS status;

    if (!device || !descriptor || !(descriptor->input_types & HID_REPORT_KEYBOARD) ||
        descriptor->report_id_count > HID_REPORT_MAX_IDS)
    {
        return EINVAL;
    }

    FOREACH_S(output_report_index, descriptor->report_id_count, HID_REPORT_MAX_IDS, 0, 1)
    {
        if (hid_report_encode_leds(descriptor, led_states, output_report_index, output_report, sizeof(output_report), &output_report_length, &report_id) != HID_PARSE_OK)
        {
            continue;
        }

        status = send_output_report(device, interface_number, output_report, output_report_length, report_id);
        if (FAIL(status))
        {
            return status;
        }
    }

    return SUCCESS;
}

/// @brief Init Lib-IOSM-USB
/// Initialize the keyboard and mouse managers before the shared USB client.
SAP_STATUS usbhid_mgr_init(void)
{
    SAP_STATUS status;
    spid_t usb2_service_id, usb3_service_id;

    if (hid_manager_initialized)
    {
        return SUCCESS;
    }

    status = usb_kbd_mgr_init();
    if (FAIL(status))
    {
        return status;
    }

    status = usb_mouse_mgr_init();
    if (FAIL(status))
    {
        usb_kbd_mgr_fini();
        return status;
    }

    usb2_service_id = sos_sys_sp_query_id(SP_SERVICE, "IOSM-USB2", true);
    usb3_service_id = sos_sys_sp_query_id(SP_SERVICE, "IOSM-USB3", true);
    status = iosm_usb_init(usb2_service_id, SP_USBDRV_USBBUF_4K_PAGES, usb3_service_id, SP_USBDRV_USBBUF_4K_PAGES);
    if (FAIL(status))
    {
        usb_mouse_mgr_fini();
        usb_kbd_mgr_fini();
        return status;
    }

    hid_manager_initialized = true;

    return SUCCESS;
}

/// @brief Finalize Lib-IOSM-USB
/// Release keyboard and mouse registrations before the shared USB client.
void usbhid_mgr_fini(void)
{
    usb_kbd_mgr_fini();
    usb_mouse_mgr_fini();
    memset(_dev_info, 0, sizeof(_dev_info));

    if (hid_manager_initialized)
    {
        iosm_usb_fini();
    }

    hid_manager_initialized = false;
}

SAP_STATUS usbhid_init_dev(struct sp_usbpdev_s *device)
{
    unsigned device_slot;
    uint32_t interface_index;
    SAP_STATUS status = SUCCESS;

    if (!device || device->data_ifaces_num > USBPDEV_MAX_DATA_INTERFACES_NUM ||
        device->data_eps_num > USBPDEV_MAX_DATA_ENDPOINTS_NUM)
    {
        return EINVAL;
    }

    FOREACH_S(device_slot, MAX_USBHID_DEV_NUM, MAX_USBHID_DEV_NUM, 0, 1)
    {
        if (_dev_info[device_slot].dev == device)
        {
            return SUCCESS;
        }
    }

    FOREACH_S(interface_index, device->data_ifaces_num, USBPDEV_MAX_DATA_INTERFACES_NUM, 0, 1)
    {
        struct usb_interface_descriptor *interface_descriptor = device->data_iface[interface_index];
        struct usbpdev_endpoint_descriptor *endpoint;
        struct hid_report_descriptor *descriptor;
        struct sp_usbpdev_pipe *pipe;
        USB_HID_TYPE device_type;
        bool boot_protocol;

        if (!interface_descriptor || interface_descriptor->bInterfaceClass != HID_CLASS)
        {
            continue;
        }

        /* Find capacity before opening a pipe. Classification must still run
         * when full, so unrelated HID interfaces can be skipped. */
        FOREACH_S(device_slot, MAX_USBHID_DEV_NUM, MAX_USBHID_DEV_NUM, 0, 1)
        {
            if (!_dev_info[device_slot].dev)
            {
                break;
            }
        }

        status = usbhid_prepare_interface(device, interface_descriptor, 0, &endpoint, &descriptor, &boot_protocol);
        if (status == E_HID_NOT_KBD_OR_MOUSE)
        {
            continue;
        }

        if (FAIL(status))
        {
            goto fail;
        }

        if (device_slot == MAX_USBHID_DEV_NUM)
        {
            free(descriptor);
            status = ENOMEM;
            goto fail;
        }

        if (descriptor->input_types == HID_REPORT_KEYBOARD)
        {
            device_type = USBHID_KBD;
            pipe = usb_kbd_init_from_descriptor(device, endpoint, descriptor, boot_protocol);
        }
        else
        {
            device_type = USBHID_MOUSE;
            pipe = usb_mouse_init_from_descriptor(device, endpoint, descriptor, boot_protocol);
        }

        /* Each init function takes ownership of descriptor, including failure. */
        if (!pipe)
        {
            status = ENOMEM;
            goto fail;
        }

        _dev_info[device_slot].dev = device;
        _dev_info[device_slot].type = device_type;
        _dev_info[device_slot].pipe = pipe;
    }

    return SUCCESS;

fail:
    usbhid_fini_dev(device);

    return status;
}

SAP_STATUS usbhid_fini_dev(struct sp_usbpdev_s *device)
{
    unsigned device_slot;

    if (!device)
    {
        return EINVAL;
    }

    FOREACH_S(device_slot, MAX_USBHID_DEV_NUM, MAX_USBHID_DEV_NUM, 0, 1)
    {
        if (_dev_info[device_slot].dev == device)
        {
            if (_dev_info[device_slot].type == USBHID_KBD)
            {
                usb_kbd_fini(_dev_info[device_slot].pipe);
            }
            else if (_dev_info[device_slot].type == USBHID_MOUSE)
            {
                usb_mouse_fini(_dev_info[device_slot].pipe);
            }

            memset(&_dev_info[device_slot], 0, sizeof(_dev_info[device_slot]));
        }
    }

    return SUCCESS;
}

SAP_STATUS usb_intr_handler(intr_t interrupt_number)
{
    SAP_STATUS keyboard_status, mouse_status;
    (void)interrupt_number;

    keyboard_status = usb_kbd_handler();
    mouse_status = usb_mouse_handler();

    return FAIL(keyboard_status) ? keyboard_status : mouse_status;
}

void usbhid_print_hid_desc(struct usb_hid_descriptor *hid_descriptor)
{
    if (hid_descriptor)
    {
        SAP_INFO("HID descriptor: type=%u report_bytes=%u\n",
            hid_descriptor->bClassDescriptorType, hid_descriptor->wClassDescriptorLength);
    }
}
