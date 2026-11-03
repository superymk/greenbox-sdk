// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers_libc.h>
#include <stdlib.h>
#include <string.h>
#include "usb-mouse.h"
#include "usbhid.h"

extern void libdrv_hid_call_mouse_listeners(struct mouse_event *event);

#define MAX_MOUSE_INTR_PIPES 8u
#define MOUSE_EVENT_CAPACITY 5u

/* One registered mouse interface owns its transport, parsed descriptor and
 * input history. An empty entry has interrupt_pipe == NULL. */
struct mouse_usb_dev
{
    struct sp_usbpdev_pipe *interrupt_pipe;
    struct hid_report_descriptor *report_descriptor;
    uint8_t interface_number;
    size_t input_packet_capacity;
    uint8_t input_report[HID_REPORT_MAX_PACKET];
    /* Each descriptor report index retains its buttons even when another
     * report ID carries only movement. pressed_buttons is their union. */
    uint8_t report_pressed_buttons[HID_REPORT_MAX_IDS];
    uint8_t pressed_buttons;
    /* A report can produce releases, presses, movement, wheel and pan. All
     * readers drain these events before polling another input report. */
    struct mouse_event pending_events[MOUSE_EVENT_CAPACITY];
    unsigned pending_event_count;
    unsigned next_pending_event_index;
};

static struct mouse_usb_dev mouse_usb_devices[MAX_MOUSE_INTR_PIPES];
/* Continue polling after the device that most recently supplied a report. */
static unsigned next_mouse_index;

static int find_mouse_device_index(struct sp_usbpdev_pipe *pipe)
{
    unsigned mouse_index;

    FOREACH_S(mouse_index, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        if (mouse_usb_devices[mouse_index].interrupt_pipe == pipe)
        {
            return (int)mouse_index;
        }
    }

    return -1;
}

/* Find queued input before any device is polled again. */
static struct mouse_usb_dev *find_mouse_with_pending_events(void)
{
    unsigned mouse_index;

    FOREACH_S(mouse_index, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        struct mouse_usb_dev *mouse_device = &mouse_usb_devices[mouse_index];

        if (mouse_device->interrupt_pipe &&
            mouse_device->next_pending_event_index < mouse_device->pending_event_count)
        {
            return mouse_device;
        }
    }

    return NULL;
}

static unsigned button_usages_to_event_buttons(uint8_t button_usages)
{
    unsigned event_button_mask = 0;

    if (button_usages & 1u)
    {
        event_button_mask |= LIBDRV_HID_BTN_LEFT;
    }

    if (button_usages & 2u)
    {
        event_button_mask |= LIBDRV_HID_BTN_RIGHT;
    }

    if (button_usages & 4u)
    {
        event_button_mask |= LIBDRV_HID_BTN_MIDDLE;
    }

    return event_button_mask;
}

static void queue_button_transition(struct mouse_usb_dev *mouse_device,
    uint8_t button_usages, bool pressed)
{
    struct mouse_event *event;

    if (!button_usages)
    {
        return;
    }

    event = &mouse_device->pending_events[mouse_device->pending_event_count++];
    event->action = pressed ? LIBDRV_HID_MOUSE_BTN_PRESSED : LIBDRV_HID_MOUSE_BTN_RELEASED;
    event->button = button_usages_to_event_buttons(button_usages);
}

static void queue_report_transitions(struct mouse_usb_dev *mouse_device, uint8_t buttons,
    uint8_t button_mask, const int32_t *axes, unsigned report_index)
{
    unsigned other_report_index;
    uint8_t combined_pressed_buttons = 0;
    struct mouse_event *event;

    if (report_index >= HID_REPORT_MAX_IDS)
    {
        return;
    }

    mouse_device->pending_event_count = 0;
    mouse_device->next_pending_event_index = 0;
    memset(mouse_device->pending_events, 0, sizeof(mouse_device->pending_events));

    mouse_device->report_pressed_buttons[report_index] =
        (mouse_device->report_pressed_buttons[report_index] & (uint8_t)~button_mask) |
        (buttons & button_mask & 7u);
    FOREACH_S(other_report_index, mouse_device->report_descriptor->report_id_count,
        HID_REPORT_MAX_IDS, 0, 1)
    {
        combined_pressed_buttons |= mouse_device->report_pressed_buttons[other_report_index];
    }

    queue_button_transition(mouse_device,
        mouse_device->pressed_buttons & (uint8_t)~combined_pressed_buttons, false);
    queue_button_transition(mouse_device,
        combined_pressed_buttons & (uint8_t)~mouse_device->pressed_buttons, true);
    mouse_device->pressed_buttons = combined_pressed_buttons;

    if (axes[HID_MOUSE_AXIS_X] || axes[HID_MOUSE_AXIS_Y])
    {
        event = &mouse_device->pending_events[mouse_device->pending_event_count++];
        event->action = LIBDRV_HID_MOUSE_MOVED;
        event->button = button_usages_to_event_buttons(combined_pressed_buttons);
        event->delta_x = axes[HID_MOUSE_AXIS_X];
        event->delta_y = axes[HID_MOUSE_AXIS_Y];
    }

    /* wheel_diff carries positive scroll distance separately from direction.
     * Widen before negation so the minimum signed 32-bit delta is safe. */
    if (axes[HID_MOUSE_AXIS_WHEEL])
    {
        event = &mouse_device->pending_events[mouse_device->pending_event_count++];
        event->action = LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button = LIBDRV_HID_BTN_WHEEL;
        event->wheel = axes[HID_MOUSE_AXIS_WHEEL] > 0 ? LIBDRV_HID_WHEEL_UP : LIBDRV_HID_WHEEL_DOWN;
        event->wheel_diff = axes[HID_MOUSE_AXIS_WHEEL] < 0 ?
            (uint32_t)(-(int64_t)axes[HID_MOUSE_AXIS_WHEEL]) : (uint32_t)axes[HID_MOUSE_AXIS_WHEEL];
    }

    if (axes[HID_MOUSE_AXIS_PAN])
    {
        event = &mouse_device->pending_events[mouse_device->pending_event_count++];
        event->action = LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button = LIBDRV_HID_BTN_WHEEL;
        event->wheel = axes[HID_MOUSE_AXIS_PAN] > 0 ? LIBDRV_HID_WHEEL_RIGHT : LIBDRV_HID_WHEEL_LEFT;
        event->wheel_diff = axes[HID_MOUSE_AXIS_PAN] < 0 ?
            (uint32_t)(-(int64_t)axes[HID_MOUSE_AXIS_PAN]) : (uint32_t)axes[HID_MOUSE_AXIS_PAN];
    }
}

static bool dequeue_mouse_event(struct mouse_usb_dev *mouse_device,
    struct mouse_event *event)
{
    if (mouse_device->next_pending_event_index == mouse_device->pending_event_count)
    {
        return false;
    }

    *event = mouse_device->pending_events[mouse_device->next_pending_event_index++];

    if (mouse_device->next_pending_event_index == mouse_device->pending_event_count)
    {
        mouse_device->next_pending_event_index = 0;
        mouse_device->pending_event_count = 0;
    }

    return true;
}

static SAP_STATUS poll_mouse_interfaces(void)
{
    SAP_STATUS first_error = SUCCESS;
    unsigned interfaces_visited;

    if (find_mouse_with_pending_events())
    {
        return SUCCESS;
    }

    FOREACH_S(interfaces_visited, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        unsigned mouse_index = next_mouse_index;
        struct mouse_usb_dev *mouse_device = &mouse_usb_devices[mouse_index];
        uint8_t buttons, button_mask;
        int32_t axes[HID_MOUSE_AXIS_COUNT];
        unsigned report_index;
        SAP_STATUS status;

        next_mouse_index = (next_mouse_index + 1u) % MAX_MOUSE_INTR_PIPES;
        if (!mouse_device->interrupt_pipe)
        {
            continue;
        }

        memset(mouse_device->input_report, 0, sizeof(mouse_device->input_report));
        status = iosm_usb_poll_intr(mouse_device->interrupt_pipe, mouse_device->input_report,
            mouse_device->input_packet_capacity);
        if (status == E_USBTD_IS_ACTIVE)
        {
            continue;
        }
        if (FAIL(status))
        {
            if (PASS(first_error))
            {
                first_error = status;
            }

            continue;
        }

        /* The SDK has no completed byte count, and xHCI copies stale DMA tail
         * bytes on short transfers. The parser receives capacity; rejecting
         * truncation requires the transport to supply the actual length. */
        if (hid_report_decode_mouse(mouse_device->report_descriptor, mouse_device->input_report,
            mouse_device->input_packet_capacity, &buttons, &button_mask, axes,
            &report_index) != HID_PARSE_OK)
        {
            continue;
        }

        queue_report_transitions(mouse_device, buttons, button_mask, axes, report_index);
        if (mouse_device->pending_event_count)
        {
            return SUCCESS;
        }
    }

    return first_error;
}

SAP_STATUS usb_mouse_mgr_init(void)
{
    usb_mouse_mgr_fini();

    return SUCCESS;
}

void usb_mouse_mgr_fini(void)
{
    unsigned mouse_index;

    FOREACH_S(mouse_index, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        usb_mouse_fini(mouse_usb_devices[mouse_index].interrupt_pipe);
    }

    next_mouse_index = 0;
}

struct sp_usbpdev_pipe *usb_mouse_init_from_descriptor(struct sp_usbpdev_s *device,
    struct usbpdev_endpoint_descriptor *endpoint, struct hid_report_descriptor *descriptor,
    bool boot_protocol)
{
    struct sp_usbpdev_pipe *interrupt_pipe = NULL;
    struct mouse_usb_dev *mouse_device;
    size_t packet_capacity;
    unsigned mouse_index;
    int pipe_index;

    if (!device || !endpoint || !endpoint->iface || !endpoint->epdesc || !descriptor ||
        descriptor->input_types != HID_REPORT_MOUSE)
    {
        goto fail;
    }

    packet_capacity = UGETW(endpoint->epdesc->wMaxPacketSize);
    if (!packet_capacity || packet_capacity > HID_REPORT_MAX_PACKET ||
        !descriptor->max_input_bytes || descriptor->max_input_bytes > packet_capacity ||
        !descriptor->report_id_count || descriptor->report_id_count > HID_REPORT_MAX_IDS)
    {
        goto fail;
    }

    FOREACH_S(mouse_index, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        mouse_device = &mouse_usb_devices[mouse_index];
        if (mouse_device->interrupt_pipe && mouse_device->interrupt_pipe->dev == device &&
            mouse_device->interface_number == endpoint->iface->bInterfaceNumber)
        {
            free(descriptor);
            return mouse_device->interrupt_pipe;
        }
    }

    pipe_index = find_mouse_device_index(NULL);
    if (pipe_index < 0)
    {
        goto fail;
    }

    interrupt_pipe = iosm_usb_usbpdev_alloc_intr_pipe(device, endpoint->epdesc);
    if (!interrupt_pipe)
    {
        goto fail;
    }

    mouse_device = &mouse_usb_devices[(unsigned)pipe_index];
    memset(mouse_device, 0, sizeof(*mouse_device));
    mouse_device->report_descriptor = descriptor;
    mouse_device->interface_number = endpoint->iface->bInterfaceNumber;
    mouse_device->input_packet_capacity = packet_capacity;
    mouse_device->interrupt_pipe = interrupt_pipe;

    SAP_INFO("USB HID mouse interface %u: running in %s protocol\n",
        mouse_device->interface_number, boot_protocol ? "BOOT" : "REPORT");
    return interrupt_pipe;

fail:
    /* A pipe allocated before registration cannot be freed by slot lookup. */
    if (interrupt_pipe)
    {
        SAP_STATUS status = iosm_usb_usbpdev_free_pipe(interrupt_pipe);

        if (FAIL(status))
        {
            SAP_ERR("USB mouse: free interrupt pipe failed (status:%lu)\n", status);
        }
    }

    free(descriptor);
    return NULL;
}

struct sp_usbpdev_pipe *usb_mouse_init(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor)
{
    struct usbpdev_endpoint_descriptor *endpoint = NULL;
    struct hid_report_descriptor *descriptor = NULL;
    bool boot_protocol = false;
    unsigned mouse_index;

    if (!device || !interface_descriptor)
    {
        return NULL;
    }

    FOREACH_S(mouse_index, MAX_MOUSE_INTR_PIPES, MAX_MOUSE_INTR_PIPES, 0, 1)
    {
        struct mouse_usb_dev *mouse_device = &mouse_usb_devices[mouse_index];

        if (mouse_device->interrupt_pipe && mouse_device->interrupt_pipe->dev == device &&
            mouse_device->interface_number == interface_descriptor->bInterfaceNumber)
        {
            return mouse_device->interrupt_pipe;
        }
    }

    if (FAIL(usbhid_prepare_interface(device, interface_descriptor, HID_REPORT_MOUSE,
        &endpoint, &descriptor, &boot_protocol)))
    {
        return NULL;
    }

    return usb_mouse_init_from_descriptor(device, endpoint, descriptor, boot_protocol);
}

void usb_mouse_fini(struct sp_usbpdev_pipe *interrupt_pipe)
{
    SAP_STATUS status;
    int pipe_index;
    struct mouse_usb_dev *mouse_device;

    if (!interrupt_pipe)
    {
        return;
    }

    pipe_index = find_mouse_device_index(interrupt_pipe);
    if (pipe_index < 0)
    {
        return;
    }

    mouse_device = &mouse_usb_devices[(unsigned)pipe_index];

    /* Unregister before releasing transport resources. Clearing this entry
     * also discards any remaining events when a listener detaches its mouse. */
    mouse_device->interrupt_pipe = NULL;
    status = iosm_usb_usbpdev_free_pipe(interrupt_pipe);
    if (FAIL(status))
    {
        SAP_ERR("USB mouse: free interrupt pipe failed (status:%lu)\n", status);
    }

    free(mouse_device->report_descriptor);
    memset(mouse_device, 0, sizeof(*mouse_device));
}

SAP_STATUS usb_mouse_read_nonblock(struct mouse_event *out_event)
{
    SAP_STATUS status;
    struct mouse_usb_dev *mouse_device;

    if (!out_event)
    {
        return EINVAL;
    }

    memset(out_event, 0, sizeof(*out_event));
    out_event->action = LIBDRV_HID_MOUSE_NO_EVENT;

    status = poll_mouse_interfaces();
    if (FAIL(status))
    {
        return status;
    }

    mouse_device = find_mouse_with_pending_events();
    if (mouse_device)
    {
        (void)dequeue_mouse_event(mouse_device, out_event);
    }

    return SUCCESS;
}

//! \brief Read mouse event due to periodically pull and/or mouse interrupts
SAP_STATUS usb_mouse_handler(void)
{
    struct mouse_event event;
    struct mouse_usb_dev *mouse_device;
    struct sp_usbpdev_pipe *source_pipe;
    SAP_STATUS status;
    unsigned event_index, events_to_deliver;

    status = poll_mouse_interfaces();
    if (FAIL(status))
    {
        return status;
    }

    mouse_device = find_mouse_with_pending_events();
    if (!mouse_device)
    {
        return SUCCESS;
    }

    source_pipe = mouse_device->interrupt_pipe;
    events_to_deliver = mouse_device->pending_event_count - mouse_device->next_pending_event_index;

    /* Copy before callbacks; a listener may detach the source pipe. Continue
     * only while this device still owns the original event queue. */
    FOREACH_S(event_index, events_to_deliver, MOUSE_EVENT_CAPACITY, 0, 1)
    {
        if (mouse_device->interrupt_pipe != source_pipe ||
            !dequeue_mouse_event(mouse_device, &event))
        {
            break;
        }

        libdrv_hid_call_mouse_listeners(&event);
    }

    return SUCCESS;
}
