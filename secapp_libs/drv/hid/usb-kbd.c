// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers_libc.h>
#include <palddk.h>
#include <stdlib.h>
#include <string.h>

#include "usb-kbd.h"
#include "usbhid.h"

extern void libdrv_hid_call_kbd_listeners(struct kbd_key_event *event);

#define MAX_KEYBOARD_INTR_PIPES 8u
#define KBD_USAGE_COUNT 256u
#define KBD_KEY_BITMAP_BYTES (KBD_USAGE_COUNT / 8u)
#define KBD_EVENT_CAPACITY KBD_USAGE_COUNT
#define LED_NUMLOCK 0x01u
#define LED_CAPSLOCK 0x02u
#define LED_SCROLLLOCK 0x04u

/* One registered keyboard interface owns its pipe, descriptor, input history,
 * and pending events. A NULL interrupt_pipe identifies an unused array slot. */
struct kbd_usb_dev
{
    struct sp_usbpdev_pipe *interrupt_pipe;
    struct hid_report_descriptor *report_descriptor;
    uint8_t interface_number;
    size_t packet_capacity;
    uint8_t input_buffer[HID_REPORT_MAX_PACKET];

    /* Each descriptor report index owns one key bitmap. Their union is the
     * held-key set, so independent report IDs cannot release each other. */
    uint8_t report_pressed_keys[HID_REPORT_MAX_IDS][KBD_KEY_BITMAP_BYTES];
    uint8_t pressed_keys[KBD_KEY_BITMAP_BYTES];
    uint8_t lock_leds;

    /* A failed control request may leave endpoint zero halted. Stop LED
     * retries until reinitialization so later lock presses do not time out. */
    bool led_updates_disabled;

    /* Drain a report's changes before polling another packet. Each usage
     * changes at most once, so the entire report fits in 256 event entries. */
    struct kbd_key_event queued_events[KBD_EVENT_CAPACITY];
    unsigned next_event_index;
    unsigned pending_event_count;
};

static struct kbd_usb_dev keyboard_devices[MAX_KEYBOARD_INTR_PIPES];
/* Resume packet polling after the last keyboard that supplied an event. */
static unsigned next_keyboard_index;

static int find_keyboard_device_index(struct sp_usbpdev_pipe *pipe)
{
    unsigned keyboard_index;

    FOREACH_S(keyboard_index, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        if (keyboard_devices[keyboard_index].interrupt_pipe == pipe)
        {
            return (int)keyboard_index;
        }
    }

    return -1;
}

static void initialize_empty_event(struct kbd_key_event *event)
{
    if (event)
    {
        memset(event, 0, sizeof(*event));
        event->action = LIBDRV_HID_KBD_NO_EVENT;
    }
}

static bool key_usage_is_pressed(const uint8_t *pressed_keys, unsigned key_usage)
{
    return (pressed_keys[key_usage / 8u] & (1u << (key_usage % 8u))) != 0;
}

static uint8_t key_usage_to_event_key(unsigned key_usage)
{
    static const uint8_t modifier_key_codes[8] = {
        LIBDRV_HID_KEY_LEFT_CTRL, LIBDRV_HID_KEY_LEFT_SHIFT,
        LIBDRV_HID_KEY_LEFT_ALT, LIBDRV_HID_KEY_LEFT_GUI,
        LIBDRV_HID_KEY_RIGHT_CTRL, LIBDRV_HID_KEY_RIGHT_SHIFT,
        LIBDRV_HID_KEY_RIGHT_ALT, LIBDRV_HID_KEY_RIGHT_GUI
    };

    if (key_usage >= 0xe0u && key_usage <= 0xe7u)
    {
        return modifier_key_codes[key_usage - 0xe0u];
    }

    /* Match the physical-key identifiers used by the PS/2 driver. The SDK
     * distinguishes square brackets/backtick from braces/tilde; Shift stays
     * in event->modifier and must not change the press/release identifier. */
    switch (key_usage)
    {
        case 0x2fu:
            return LIBDRV_HID_KEY_LEFT_SQUARE;
        case 0x30u:
            return LIBDRV_HID_KEY_RIGHT_SQUARE;
        case 0x35u:
            return LIBDRV_HID_KEY_BACKTICK;
        default:
            return (uint8_t)key_usage;
    }
}

static uint32_t pressed_key_modifiers(const uint8_t *pressed_keys)
{
    static const uint32_t modifier_flags[8] = {
        LIBDRV_HID_MOD_LEFT_CTRL, LIBDRV_HID_MOD_LEFT_SHIFT,
        LIBDRV_HID_MOD_LEFT_ALT, LIBDRV_HID_MOD_LEFT_GUI,
        LIBDRV_HID_MOD_RIGHT_CTRL, LIBDRV_HID_MOD_RIGHT_SHIFT,
        LIBDRV_HID_MOD_RIGHT_ALT, LIBDRV_HID_MOD_RIGHT_GUI
    };
    uint32_t modifier_mask = 0;
    unsigned modifier_index;

    FOREACH_S(modifier_index, sizeof(modifier_flags) / sizeof(modifier_flags[0]), 8u, 0, 1)
    {
        if (key_usage_is_pressed(pressed_keys, 0xe0u + modifier_index))
        {
            modifier_mask |= modifier_flags[modifier_index];
        }
    }

    return modifier_mask;
}

static uint32_t lock_state_modifiers(uint8_t lock_leds)
{
    uint32_t modifier_mask = 0;

    if (lock_leds & LED_NUMLOCK)
    {
        modifier_mask |= LIBDRV_HID_MOD_NUM_LOCK;
    }

    if (lock_leds & LED_CAPSLOCK)
    {
        modifier_mask |= LIBDRV_HID_MOD_CAPS_LOCK;
    }

    if (lock_leds & LED_SCROLLLOCK)
    {
        modifier_mask |= LIBDRV_HID_MOD_SCROLL_LOCK;
    }

    return modifier_mask;
}

/* LED transport failure must not disable software lock state or key events.
 * The first failed request can still wait for the USB control timeout. */
static void update_keyboard_leds(struct kbd_usb_dev *keyboard)
{
    SAP_STATUS status;

    if (keyboard->led_updates_disabled)
    {
        return;
    }

    status = usbhid_send_leds(keyboard->interrupt_pipe->dev,
        keyboard->interface_number, keyboard->report_descriptor, keyboard->lock_leds);
    if (FAIL(status))
    {
        keyboard->led_updates_disabled = true;
        SAP_ERR("USB HID keyboard interface %u: LED SET_REPORT failed (status:%lu); "
            "LED updates disabled until reinitialization\n", keyboard->interface_number, status);
    }
}

static void queue_key_transition(struct kbd_usb_dev *keyboard, unsigned key_usage,
    uint8_t action, uint32_t modifier_mask)
{
    struct kbd_key_event *event = &keyboard->queued_events[keyboard->pending_event_count++];

    initialize_empty_event(event);
    event->key = key_usage_to_event_key(key_usage);
    event->action = action;

    if (action == LIBDRV_HID_KBD_KEY_PRESSED)
    {
        if (event->key == LIBDRV_HID_KEY_NUM_LOCK)
        {
            keyboard->lock_leds ^= LED_NUMLOCK;
        }
        else if (event->key == LIBDRV_HID_KEY_CAPS_LOCK)
        {
            keyboard->lock_leds ^= LED_CAPSLOCK;
        }
        else if (event->key == LIBDRV_HID_KEY_SCROLL_LOCK)
        {
            keyboard->lock_leds ^= LED_SCROLLLOCK;
        }
    }

    event->modifier = modifier_mask | lock_state_modifiers(keyboard->lock_leds);
}

static void queue_report_transitions(struct kbd_usb_dev *keyboard,
    const uint8_t *report_keys, unsigned report_index)
{
    uint8_t combined_pressed_keys[KBD_KEY_BITMAP_BYTES];
    uint8_t previous_lock_leds = keyboard->lock_leds;
    uint32_t modifier_mask;
    unsigned other_report_index;
    unsigned key_byte_index;
    unsigned key_usage;

    /* ErrorRollOver, POSTFail and ErrorUndefined leave held keys unchanged. */
    if (report_index >= keyboard->report_descriptor->report_id_count ||
        report_index >= HID_REPORT_MAX_IDS || (report_keys[0] & 0x0eu))
    {
        return;
    }

    memcpy(combined_pressed_keys, report_keys, sizeof(combined_pressed_keys));
    combined_pressed_keys[0] &= 0xf0u;
    FOREACH_S(other_report_index, keyboard->report_descriptor->report_id_count, HID_REPORT_MAX_IDS, 0, 1)
    {
        if (other_report_index != report_index)
        {
            FOREACH_S(key_byte_index, sizeof(combined_pressed_keys), KBD_KEY_BITMAP_BYTES, 0, 1)
            {
                combined_pressed_keys[key_byte_index] |=
                    keyboard->report_pressed_keys[other_report_index][key_byte_index];
            }
        }
    }

    keyboard->next_event_index = 0;
    keyboard->pending_event_count = 0;
    modifier_mask = pressed_key_modifiers(combined_pressed_keys);

    /* Replacements release old keys before pressing new ones. Modifier bits
     * describe the new union, including independent modifier report IDs. */
    FOREACH_S(key_usage, KBD_USAGE_COUNT, KBD_USAGE_COUNT, 4, 1)
    {
        if (key_usage_is_pressed(keyboard->pressed_keys, key_usage) &&
            !key_usage_is_pressed(combined_pressed_keys, key_usage))
        {
            queue_key_transition(keyboard, key_usage, LIBDRV_HID_KBD_KEY_RELEASED, modifier_mask);
        }
    }

    FOREACH_S(key_usage, KBD_USAGE_COUNT, KBD_USAGE_COUNT, 4, 1)
    {
        if (!key_usage_is_pressed(keyboard->pressed_keys, key_usage) &&
            key_usage_is_pressed(combined_pressed_keys, key_usage))
        {
            queue_key_transition(keyboard, key_usage, LIBDRV_HID_KBD_KEY_PRESSED, modifier_mask);
        }
    }

    memcpy(keyboard->report_pressed_keys[report_index], report_keys, KBD_KEY_BITMAP_BYTES);
    keyboard->report_pressed_keys[report_index][0] &= 0xf0u;
    memcpy(keyboard->pressed_keys, combined_pressed_keys, KBD_KEY_BITMAP_BYTES);

    if (previous_lock_leds != keyboard->lock_leds)
    {
        update_keyboard_leds(keyboard);
    }
}

static bool dequeue_key_event(struct kbd_usb_dev *keyboard, struct kbd_key_event *event)
{
    if (!keyboard || !keyboard->interrupt_pipe || !keyboard->pending_event_count)
    {
        return false;
    }

    *event = keyboard->queued_events[keyboard->next_event_index++];
    if (!--keyboard->pending_event_count)
    {
        keyboard->next_event_index = 0;
    }

    return true;
}

/* Visit every pipe at most once, resuming after the last pipe served. */
static SAP_STATUS poll_keyboard_interfaces(struct kbd_usb_dev **output_keyboard)
{
    SAP_STATUS first_error = SUCCESS;
    unsigned interfaces_visited;
    unsigned keyboard_index;

    *output_keyboard = NULL;

    /* Reads return only one or two events. Keep the remaining events with
     * their device and finish that report before accepting another packet. */
    FOREACH_S(keyboard_index, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        if (keyboard_devices[keyboard_index].interrupt_pipe &&
            keyboard_devices[keyboard_index].pending_event_count)
        {
            *output_keyboard = &keyboard_devices[keyboard_index];
            return SUCCESS;
        }
    }

    FOREACH_S(interfaces_visited, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        struct kbd_usb_dev *keyboard = &keyboard_devices[next_keyboard_index];
        uint8_t report_keys[KBD_KEY_BITMAP_BYTES];
        unsigned report_index;
        SAP_STATUS status;

        next_keyboard_index = (next_keyboard_index + 1u) % MAX_KEYBOARD_INTR_PIPES;
        if (!keyboard->interrupt_pipe)
        {
            continue;
        }

        memset(keyboard->input_buffer, 0, sizeof(keyboard->input_buffer));
        status = iosm_usb_poll_intr(keyboard->interrupt_pipe, keyboard->input_buffer,
            keyboard->packet_capacity);
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
        if (hid_report_decode_keyboard(keyboard->report_descriptor, keyboard->input_buffer,
            keyboard->packet_capacity, report_keys, &report_index) != HID_PARSE_OK)
        {
            continue;
        }

        queue_report_transitions(keyboard, report_keys, report_index);
        if (keyboard->pending_event_count)
        {
            *output_keyboard = keyboard;
            return SUCCESS;
        }
    }

    return first_error;
}

SAP_STATUS usb_kbd_mgr_init(void)
{
    usb_kbd_mgr_fini();

    return SUCCESS;
}

void usb_kbd_mgr_fini(void)
{
    unsigned keyboard_index;

    FOREACH_S(keyboard_index, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        usb_kbd_fini(keyboard_devices[keyboard_index].interrupt_pipe);
    }

    next_keyboard_index = 0;
}

struct sp_usbpdev_pipe *usb_kbd_init_from_descriptor(struct sp_usbpdev_s *device,
    struct usbpdev_endpoint_descriptor *endpoint, struct hid_report_descriptor *descriptor,
    bool boot_protocol)
{
    struct sp_usbpdev_pipe *interrupt_pipe = NULL;
    size_t packet_capacity;
    unsigned keyboard_index;
    int pipe_index;
    struct kbd_usb_dev *keyboard;

    if (!device || !endpoint || !endpoint->iface || !endpoint->epdesc || !descriptor ||
        descriptor->input_types != HID_REPORT_KEYBOARD)
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

    FOREACH_S(keyboard_index, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        if (keyboard_devices[keyboard_index].interrupt_pipe &&
            keyboard_devices[keyboard_index].interrupt_pipe->dev == device &&
            keyboard_devices[keyboard_index].interface_number == endpoint->iface->bInterfaceNumber)
        {
            free(descriptor);
            return keyboard_devices[keyboard_index].interrupt_pipe;
        }
    }

    pipe_index = find_keyboard_device_index(NULL);
    if (pipe_index < 0)
    {
        goto fail;
    }

    interrupt_pipe = iosm_usb_usbpdev_alloc_intr_pipe(device, endpoint->epdesc);
    if (!interrupt_pipe)
    {
        goto fail;
    }

    keyboard_index = (unsigned)pipe_index;
    keyboard = &keyboard_devices[keyboard_index];
    memset(keyboard, 0, sizeof(*keyboard));
    keyboard->report_descriptor = descriptor;
    keyboard->interface_number = endpoint->iface->bInterfaceNumber;
    keyboard->packet_capacity = packet_capacity;
    keyboard->interrupt_pipe = interrupt_pipe;

    update_keyboard_leds(keyboard);

    SAP_INFO("USB HID keyboard interface %u: running in %s protocol\n",
        keyboard->interface_number, boot_protocol ? "BOOT" : "REPORT");

    return interrupt_pipe;

fail:
    /* The allocation may not yet have a registry slot. Free it directly. */
    if (interrupt_pipe)
    {
        SAP_STATUS status = iosm_usb_usbpdev_free_pipe(interrupt_pipe);

        if (FAIL(status))
        {
            SAP_ERR("USB keyboard: free interrupt pipe failed (status:%lu)\n", status);
        }
    }

    free(descriptor);
    return NULL;
}

struct sp_usbpdev_pipe *usb_kbd_init(struct sp_usbpdev_s *device,
    struct usb_interface_descriptor *interface_descriptor)
{
    struct usbpdev_endpoint_descriptor *endpoint = NULL;
    struct hid_report_descriptor *descriptor = NULL;
    bool boot_protocol = false;
    unsigned keyboard_index;

    if (!device || !interface_descriptor)
    {
        return NULL;
    }

    FOREACH_S(keyboard_index, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        if (keyboard_devices[keyboard_index].interrupt_pipe &&
            keyboard_devices[keyboard_index].interrupt_pipe->dev == device &&
            keyboard_devices[keyboard_index].interface_number == interface_descriptor->bInterfaceNumber)
        {
            return keyboard_devices[keyboard_index].interrupt_pipe;
        }
    }

    if (FAIL(usbhid_prepare_interface(device, interface_descriptor, HID_REPORT_KEYBOARD,
        &endpoint, &descriptor, &boot_protocol)))
    {
        return NULL;
    }

    return usb_kbd_init_from_descriptor(device, endpoint, descriptor, boot_protocol);
}

void usb_kbd_fini(struct sp_usbpdev_pipe *interrupt_pipe)
{
    SAP_STATUS status;
    int pipe_index;
    struct kbd_usb_dev *keyboard;

    if (!interrupt_pipe)
    {
        return;
    }

    pipe_index = find_keyboard_device_index(interrupt_pipe);
    if (pipe_index < 0)
    {
        return;
    }

    keyboard = &keyboard_devices[pipe_index];
    keyboard->interrupt_pipe = NULL;

    status = iosm_usb_usbpdev_free_pipe(interrupt_pipe);
    if (FAIL(status))
    {
        SAP_ERR("USB keyboard: free interrupt pipe failed (status:%lu)\n", status);
    }

    free(keyboard->report_descriptor);
    memset(keyboard, 0, sizeof(*keyboard));
}

SAP_STATUS usb_kbd_read_nonblock(struct kbd_key_event *out_1st_key_event,
    struct kbd_key_event *out_2nd_key_event)
{
    SAP_STATUS status;
    struct kbd_usb_dev *keyboard = NULL;

    initialize_empty_event(out_1st_key_event);
    initialize_empty_event(out_2nd_key_event);

    if (!out_1st_key_event || !out_2nd_key_event)
    {
        return EINVAL;
    }

    status = poll_keyboard_interfaces(&keyboard);
    if (FAIL(status))
    {
        return status;
    }

    (void)dequeue_key_event(keyboard, out_1st_key_event);
    (void)dequeue_key_event(keyboard, out_2nd_key_event);
    return SUCCESS;
}

//! Read keyboard input. The function blocks if there is no key pressed/released.
SAP_STATUS usb_kbd_read(struct kbd_key_event *out_key_event)
{
    SAP_STATUS status;
    struct kbd_usb_dev *keyboard = NULL;

    initialize_empty_event(out_key_event);

    if (!out_key_event)
    {
        return EINVAL;
    }

    // Block if there is no kbd input read.
    while (true)
    {
        status = poll_keyboard_interfaces(&keyboard);
        if (FAIL(status))
        {
            return status;
        }

        if (dequeue_key_event(keyboard, out_key_event))
        {
            return SUCCESS;
        }
    }
}

//! \brief Read keyboard event due to periodically pull and/or mouse interrupts
SAP_STATUS usb_kbd_handler(void)
{
    struct kbd_key_event event;
    struct kbd_usb_dev *keyboard = NULL;
    SAP_STATUS status;
    unsigned event_index;
    unsigned events_to_deliver;

    status = poll_keyboard_interfaces(&keyboard);
    if (FAIL(status))
    {
        return status;
    }

    events_to_deliver = keyboard ? keyboard->pending_event_count : 0;

    /* Copy before callbacks; a listener may detach the source pipe. */
    FOREACH_S(event_index, events_to_deliver, KBD_EVENT_CAPACITY, 0, 1)
    {
        if (!dequeue_key_event(keyboard, &event))
        {
            break;
        }

        libdrv_hid_call_kbd_listeners(&event);
    }

    return SUCCESS;
}
