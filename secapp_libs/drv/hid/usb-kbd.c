// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers_libc.h>
#include <palddk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib-iosm-usb.h>

#include "debug.h"
#include "usb-kbd.h"
#include "usbhid.h"

extern void libdrv_hid_kbd_event_enqueue(struct kbd_key_event *event);
extern void libdrv_hid_call_kbd_listeners(struct kbd_key_event *event);

static void _kbd_free_intrpipe(struct sp_usbpdev_pipe *pipe);

#define MAX_KEYBOARD_INTR_PIPES (8)

static struct sp_usbpdev_pipe *_kbd_intr_pipes[MAX_KEYBOARD_INTR_PIPES] = {0};

static char *kbd_keydata = NULL;
// static bool kbd_isr_enable = false;

static bool capslock = false;
static bool numlock = false;
static bool scrollock = false;

/********* Private Functions *********/
static void _pipe_infos_clean(void)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        _kbd_intr_pipes[i] = NULL;
    }
}

static int _pipe_infos_find(struct sp_usbpdev_pipe *pipe)
{
    uint32_t i = 0;

    FOREACH_S(i, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        if (_kbd_intr_pipes[i] == pipe)
            return i;
    }

    return -1;
}

// Find the first endpoint of a given type in an interface description.
struct usb_endpoint_descriptor *_kbd_find_ep(
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

#define LED_NUMLOCK (0x1)
#define LED_CAPSLOCK (0x2)
#define LED_SCROLLLOCK (0x4)

static SAP_STATUS _send_led(struct sp_usbpdev_s *usbkbd_dev, uint8_t led)
{
    SAP_STATUS status = SUCCESS;
    struct usb_device_request *req = NULL;
    uint8_t *data = NULL;

    req = (struct usb_device_request *)usbbuf_alloc(USBBUF_MEM_DMA, sizeof(struct usb_device_request));
    if (!req)
    {
        status = ENOMEM;
        goto mem_err;
    }

    data = (uint8_t *)usbbuf_alloc(USBBUF_MEM_DMA, 1);
    if (!data)
    {
        status = ENOMEM;
        goto mem_err;
    }
    *data = led;

    req->bmRequestType = UT_WRITE_CLASS_INTERFACE;
    req->bRequest = 9;
    USETW(req->wValue, 0x0200);
    USETW(req->wIndex, 0);
    USETW(req->wLength, 1);

    status = iosm_usb_send_default_control(usbkbd_dev, req, data, USB_TRANSFER_DEFAULT_TIMEOUT_MS);
    usbbuf_dealloc(USBBUF_MEM_DMA, data);
    usbbuf_dealloc(USBBUF_MEM_DMA, req);
    if (FAIL(status))
        return EINTERNAL;

    // On success
    return SUCCESS;

mem_err:
    if (data)
        usbbuf_dealloc(USBBUF_MEM_DMA, data);
    if (req)
        usbbuf_dealloc(USBBUF_MEM_DMA, req);
    return status;
}

/********* Public Functions *********/
#define KBD_KEYDATA_SIZE (8)
SAP_STATUS usb_kbd_mgr_init(void)
{
    _pipe_infos_clean();

    // Allocate kbd_keydata
    if (!kbd_keydata)
    {
        kbd_keydata = (char *)malloc(KBD_KEYDATA_SIZE);
        if (!kbd_keydata)
        {
            SAP_ERR("Allocate USB keyboard data error!\n");
            return ENOMEM;
        }
    }

    // On success
    return SUCCESS;
}

void usb_kbd_mgr_fini(void)
{
    if (kbd_keydata)
    {
        free(kbd_keydata);
        kbd_keydata = NULL;
    }

    _pipe_infos_clean();
}

struct sp_usbpdev_pipe *usb_kbd_init(struct sp_usbpdev_s *usbkbd_dev, struct usb_interface_descriptor *iface)
{
    SAP_STATUS status = SUCCESS;
    struct sp_usbpdev_pipe *_usbkbd_intr_pipe = NULL;

    // Sanity checks
    if (!usbkbd_dev || !iface)
    {
        SAP_ERR("<usb_kbd_init> usbkbd_dev has not been initialized yet!\n");
        return NULL;
    }

    // Init the interrupt pipe
    struct usb_endpoint_descriptor *epdesc = _kbd_find_ep(
        usbkbd_dev, USB_ENDPOINT_XFER_INT, UE_DIR_IN, iface);
    if (!epdesc)
    {
        SAP_ERR("epdesc is NULL!\n");
        goto get_ep_desc_err;
    }
    
    if (UGETW(epdesc->wMaxPacketSize) < 8)
    {
        SAP_ERR("Unsupported wMaxPacketSize (%u) of the keyboard!\n", UGETW(epdesc->wMaxPacketSize));
        goto get_ep_desc_err;
    }

    SAP_INFO("<usb_kbd_init>: Alloc Keyboard Interrupt Pipe\n");
    _usbkbd_intr_pipe = iosm_usb_usbpdev_alloc_intr_pipe(usbkbd_dev, epdesc);
    if (!_usbkbd_intr_pipe)
    {
        SAP_ERR("Allocate USB keyboard interrupt pipe error!\n");
        goto alloc_intr_pipe_err;
    }
    SAP_INFO("Finish allocating keyboard interrupt pipe\n");

    if (usbkbd_dev->bus_type == USB3)
    {
        // [TODO] Use BOOT protocol for now. We should switch to REPORT protocol
        if (iface->bInterfaceSubClass == 1)
        {
            // The device supports BOOT protocol, see "Detecting HID devices" on https://wiki.osdev.org/USB_Human_Interface_Devices
            status = set_protocol(usbkbd_dev, 0, iface->bInterfaceNumber);
            if (FAIL(status))
            {
                SAP_ERR("USB Keyboard init HID error: failed to set protocol! status:%lu\n", status);
                goto usb_hid_err;
            }

            status = set_idle(usbkbd_dev, 0);
            if (FAIL(status))
            {
                // [TODO][Quirk] Some devices (e.g., Logitech G502) cannot handle <set_idle> here. So this function
                // continues on error.
                SAP_ERR("USB Keyboard init HID error: failed to set idle! status:%lu\n", status);
                // goto usb_hid_err;
            }
        }
        else
        {
            SAP_ERR("USB Keyboard init HID error: Device (interface:%u) does not support BOOT protocol!\n", iface->bInterfaceNumber);
            goto usb_hid_err;
        }
    }

    // // Enable ISR
    // kbd_isr_enable = true;

    // Record the interrupt pipe
    {
        int idx = _pipe_infos_find(NULL);

        if (idx == -1)
            goto record_pipe_err;

        _kbd_intr_pipes[idx] = _usbkbd_intr_pipe;
    }

    // Reset keyboard lights. Only NUMLOCK is on.
    status = _send_led(usbkbd_dev, LED_NUMLOCK);
    if (FAIL(status))
        SAP_ERR("USB Keyboard init HID error: failed to clear lights!\n");

    // On success
    return _usbkbd_intr_pipe;

record_pipe_err:
usb_hid_err:
    if (_usbkbd_intr_pipe)
    {
        _kbd_free_intrpipe(_usbkbd_intr_pipe);
        _usbkbd_intr_pipe = NULL;
    }

alloc_intr_pipe_err:
get_ep_desc_err:
    return NULL;
}

void usb_kbd_fini(struct sp_usbpdev_pipe *pipe)
{
    // // Disable ISR
    // kbd_isr_enable = false;

    // Free kbd USB intr pipe
    _kbd_free_intrpipe(pipe);

    SAP_INFO("Finalized USB Keyboard Specific USB pipes %p\n", pipe);
}

#define USBKBD_MOD_LEFT_CTRL 0x001
#define USBKBD_MOD_LEFT_SHIFT 0x002
#define USBKBD_MOD_LEFT_ALT 0x004
#define USBKBD_MOD_LEFT_GUI 0x008
#define USBKBD_MOD_RIGHT_CTRL 0x010
#define USBKBD_MOD_RIGHT_SHIFT 0x020
#define USBKBD_MOD_RIGHT_ALT 0x040
#define USBKBD_MOD_RIGHT_GUI 0x080

static void _get_key_of_one_modifier_diff(uint8_t diff, uint8_t *out_key)
{
    if (diff & USBKBD_MOD_LEFT_CTRL)
        *out_key = LIBDRV_HID_KEY_LEFT_CTRL;
    else if (diff & USBKBD_MOD_LEFT_SHIFT)
        *out_key = LIBDRV_HID_KEY_LEFT_SHIFT;
    else if (diff & USBKBD_MOD_LEFT_ALT)
        *out_key = LIBDRV_HID_KEY_LEFT_ALT;
    else if (diff & USBKBD_MOD_LEFT_GUI)
        *out_key = LIBDRV_HID_KEY_LEFT_GUI;
    else if (diff & USBKBD_MOD_RIGHT_CTRL)
        *out_key = LIBDRV_HID_KEY_RIGHT_CTRL;
    else if (diff & USBKBD_MOD_RIGHT_SHIFT)
        *out_key = LIBDRV_HID_KEY_RIGHT_SHIFT;
    else if (diff & USBKBD_MOD_RIGHT_ALT)
        *out_key = LIBDRV_HID_KEY_RIGHT_ALT;
    else if (diff & USBKBD_MOD_RIGHT_GUI)
        *out_key = LIBDRV_HID_KEY_RIGHT_GUI;
}

#define USBKBD_KEY_NO_SCANCODE (0)
#define USBKBD_KEY_PHANTOM_CONDITION (1)
#define USBKBD_KEY_SELF_TEST_FAILURE (2)
#define USBKBD_KEY_UNDEFINED_ERROR (3)

#define USBKBD_KEY_INVALID (0xFF)

/// @brief Compare the new keyboard input values <usbkbd_modifier> and <usbkbd_character> with their history and output
/// the corresponding one or two keyboard events.
/// [NOTE] This function may return two events if <usbkbd_character> (i.e., keyboard data at byte [2]) is different from
/// its history and both of them are not USBKBD_KEY_NO_SCANCODE, see https://wiki.osdev.org/USB_Human_Interface_Devices
/// [TODO] We assume that users always newly press or release only one modifier key.
/// @param usbkbd_last_modifier
/// @param usbkbd_last_character
/// @param usbkbd_modifier
/// @param usbkbd_character
/// @param out_1st_action
/// @param out_1st_key
/// @param out_2nd_action
/// @param out_2nd_key
static void _parse_kbd_input(uint8_t usbkbd_last_modifier, uint8_t usbkbd_last_character, uint8_t usbkbd_modifier, uint8_t usbkbd_character, uint8_t *out_1st_action, uint8_t *out_1st_key, uint8_t *out_2nd_action, uint8_t *out_2nd_key)
{
    // If the last key modifiers and keypress are not changed, then there is no keypress changes
    if ((usbkbd_last_modifier == usbkbd_modifier) && (usbkbd_last_character == usbkbd_character))
    {
        *out_1st_action = LIBDRV_HID_KBD_NO_EVENT;
        *out_1st_key = USBKBD_KEY_NO_SCANCODE;

        *out_2nd_action = LIBDRV_HID_KBD_NO_EVENT;

        return;
    }

    // Handle error report in <usbkbd_character>
    if ((usbkbd_character == USBKBD_KEY_PHANTOM_CONDITION) || (usbkbd_character == USBKBD_KEY_SELF_TEST_FAILURE) || (usbkbd_character == USBKBD_KEY_UNDEFINED_ERROR))
    {
        *out_1st_action = LIBDRV_HID_KBD_NO_EVENT;
        *out_1st_key = USBKBD_KEY_NO_SCANCODE;

        *out_2nd_action = LIBDRV_HID_KBD_NO_EVENT;

        return;
    }

    // Handle press/release of modifiers.
    // [TODO] We assume one modifier key press/release at a time only
    if (usbkbd_last_modifier < usbkbd_modifier)
    {
        // A modifier is pressed
        uint8_t diff = usbkbd_last_modifier ^ usbkbd_modifier;

        *out_1st_action = LIBDRV_HID_KBD_KEY_PRESSED;
        _get_key_of_one_modifier_diff(diff, out_1st_key);

        *out_2nd_action = LIBDRV_HID_KBD_NO_EVENT;

        return;
    }
    else if (usbkbd_last_modifier > usbkbd_modifier)
    {
        // A modifier is released
        uint8_t diff = usbkbd_last_modifier ^ usbkbd_modifier;

        *out_1st_action = LIBDRV_HID_KBD_KEY_RELEASED;
        _get_key_of_one_modifier_diff(diff, out_1st_key);

        *out_2nd_action = LIBDRV_HID_KBD_NO_EVENT;

        return;
    }

    // Handle press/release of keys
    // The keys are queued in keyboard data from byte 2 to byte 7. This function checks <usbkbd_character> (i.e., byte 2)
    // only. If there are further bytes in byte 3 to byte 7, they will overwrite <usbkbd_character> in the next and this
    // function can handle them by observing changes in <usbkbd_character>.
    if ((usbkbd_last_character == USBKBD_KEY_NO_SCANCODE) && (usbkbd_character != USBKBD_KEY_NO_SCANCODE))
    {
        // A key is pressed
        *out_1st_action = LIBDRV_HID_KBD_KEY_PRESSED;
        *out_1st_key = usbkbd_character;
        *out_2nd_action = LIBDRV_HID_KBD_NO_EVENT;

        return;
    }
    else if ((usbkbd_character == USBKBD_KEY_NO_SCANCODE) && (usbkbd_last_character != USBKBD_KEY_NO_SCANCODE))
    {
        // A key is released
        *out_1st_action = LIBDRV_HID_KBD_KEY_RELEASED;
        *out_1st_key = usbkbd_last_character;
        *out_2nd_action = LIBDRV_HID_KBD_NO_EVENT;

        return;
    }
    // No need to check "usbkbd_last_character == 0 && usbkbd_character == 0", because either a modifier is changed or
    // there is no event at all. The function handles both cases earlier.
    else if (usbkbd_character != usbkbd_last_character)
    {
        // The previous key is released and a new key is pressed.
        *out_1st_action = LIBDRV_HID_KBD_KEY_RELEASED;
        *out_1st_key = usbkbd_last_character;

        *out_2nd_action = LIBDRV_HID_KBD_KEY_PRESSED;
        *out_2nd_key = usbkbd_character;

        return;
    }

    return;
}

static uint8_t last_modifier = 0;
static uint8_t last_character = USBKBD_KEY_NO_SCANCODE;
static char led_packet = 0x0;
static SAP_STATUS _kbd_read_nonblock(struct sp_usbpdev_pipe *pipe, struct kbd_key_event *out_1st_key_event, struct kbd_key_event *out_2nd_key_event)
{
    SAP_STATUS status = SUCCESS;
    // int ret = 0;
    uint8_t modifier = 0;
    uint8_t character = 0;
    uint8_t key_action1 = LIBDRV_HID_KBD_NO_EVENT;
    uint8_t key_action2 = LIBDRV_HID_KBD_NO_EVENT;
    struct kbd_key_event key_event = {.action = LIBDRV_HID_KBD_NO_EVENT};

    // Sanity checks
    if (!pipe)
    {
        *out_1st_key_event = key_event;
        return EINVAL;
    }

    {
        for (;;)
        {
            status = iosm_usb_poll_intr(pipe, kbd_keydata, KBD_KEYDATA_SIZE);
            if (PASS(status))
                // Received key press/release
                break;
            else if (status == E_USBTD_IS_ACTIVE)
            {
                status = SUCCESS;
                key_event.action = LIBDRV_HID_KBD_NO_EVENT;
                goto out;
            }
            else
            {
                SAP_ERR("QTD error! status:%lu\n", status);
                status = EINTERNAL;
                key_event.action = LIBDRV_HID_KBD_NO_EVENT;
                goto out;
            }
        }

        modifier = kbd_keydata[0];
        character = kbd_keydata[2];
        memset(kbd_keydata, 0, KBD_KEYDATA_SIZE);

        if (modifier)
        {
            uint32_t key_modifier = 0;

            if (modifier & USBKBD_MOD_LEFT_CTRL)
                key_modifier |= LIBDRV_HID_MOD_LEFT_CTRL;
            if (modifier & USBKBD_MOD_LEFT_SHIFT)
                key_modifier |= LIBDRV_HID_MOD_LEFT_SHIFT;
            if (modifier & USBKBD_MOD_LEFT_ALT)
                key_modifier |= LIBDRV_HID_MOD_LEFT_ALT;
            if (modifier & USBKBD_MOD_LEFT_GUI)
                key_modifier |= LIBDRV_HID_MOD_LEFT_GUI;
            if (modifier & USBKBD_MOD_RIGHT_CTRL)
                key_modifier |= LIBDRV_HID_MOD_RIGHT_CTRL;
            if (modifier & USBKBD_MOD_RIGHT_SHIFT)
                key_modifier |= LIBDRV_HID_MOD_RIGHT_SHIFT;
            if (modifier & USBKBD_MOD_RIGHT_ALT)
                key_modifier |= LIBDRV_HID_MOD_RIGHT_ALT;
            if (modifier & USBKBD_MOD_RIGHT_GUI)
                key_modifier |= LIBDRV_HID_MOD_RIGHT_GUI;

            key_event.modifier = key_modifier;
        }

        _parse_kbd_input(last_modifier, last_character, modifier, character, &key_action1, &key_event.key, &key_action2, &out_2nd_key_event->key);
        key_event.action = key_action1;
        out_2nd_key_event->action = key_action2;

        // Handle CAPSLOCK, NUMLOCK, SCREENLOCK
        if (key_event.action == LIBDRV_HID_KBD_KEY_PRESSED)
        {
            // caps lock
            if (key_event.key == LIBDRV_HID_KEY_CAPS_LOCK)
            {
                if (capslock)
                {
                    capslock = false;
                    led_packet &= ~LED_CAPSLOCK;
                }
                else
                {
                    capslock = true;
                    led_packet |= LED_CAPSLOCK;
                }

                status = _send_led(pipe->dev, led_packet);
                if (FAIL(status))
                    SAP_ERR("Update CAPSLOCK light error! led_packet:%u\n", led_packet);
            }

            // num lock
            if (key_event.key == LIBDRV_HID_KEY_NUM_LOCK)
            {
                if (numlock)
                {
                    numlock = false;
                    led_packet &= ~LED_NUMLOCK;
                }
                else
                {
                    numlock = true;
                    led_packet |= LED_NUMLOCK;
                }

                status = _send_led(pipe->dev, led_packet);
                if (FAIL(status))
                    SAP_ERR("Update NUMLOCK light error! led_packet:%u\n", led_packet);
            }

            // scroll lock
            if (key_event.key == LIBDRV_HID_KEY_SCROLL_LOCK)
            {
                if (scrollock)
                {
                    scrollock = false;
                    led_packet &= ~LED_SCROLLLOCK;
                }
                else
                {
                    scrollock = true;
                    led_packet |= LED_SCROLLLOCK;
                }

                status = _send_led(pipe->dev, led_packet);
                if (FAIL(status))
                    SAP_ERR("Update SCROLLLOCK light error! led_packet:%u\n", led_packet);
            }
        }

        // Apply CAPSLOCK, NUMLOCK, SCREENLOCK modifiers to subsequent key presses/releases
        if (key_event.action != LIBDRV_HID_KBD_NO_EVENT)
        {
            if (capslock)
                key_event.modifier |= LIBDRV_HID_MOD_CAPS_LOCK;
            else
                key_event.modifier &= ~(LIBDRV_HID_MOD_CAPS_LOCK);

            if (numlock)
                key_event.modifier |= LIBDRV_HID_MOD_NUM_LOCK;
            else
                key_event.modifier &= ~(LIBDRV_HID_MOD_NUM_LOCK);

            if (scrollock)
                key_event.modifier |= LIBDRV_HID_MOD_SCROLL_LOCK;
            else
                key_event.modifier &= ~(LIBDRV_HID_MOD_SCROLL_LOCK);
        }

        out_2nd_key_event->modifier = key_event.modifier;

        // Update <last_modifier> and <last_character> according to the key events parsed in <_parse_kbd_input>

        // if(!*out_has_next_key)
        if (out_2nd_key_event->action == LIBDRV_HID_KBD_NO_EVENT)
        {
            // If there is only one key event, then update the last event to be the current event
            last_modifier = modifier;
            last_character = character;
        }
        else if (out_2nd_key_event->action == LIBDRV_HID_KBD_KEY_PRESSED)
        {
            // If there is follow-up key event, then <last_character> must be an invalid value to handle the next
            // <character>.
            // [NOTE] The function must not set <last_character> to USBKBD_KEY_NO_SCANCODE. Otherwise, the function
            // misses key releases that have no modifiers.
            last_modifier = modifier;
            // last_character = USBKBD_KEY_INVALID;
            last_character = out_2nd_key_event->key;
        }
    }

    // On success
    status = SUCCESS;

out:
    *out_1st_key_event = key_event;
    return status;
}

SAP_STATUS usb_kbd_read_nonblock(struct kbd_key_event *out_1st_key_event, struct kbd_key_event *out_2nd_key_event)
{
    SAP_STATUS status = SUCCESS;
    uint32_t i = 0;

    if (!out_1st_key_event || !out_2nd_key_event)
        return EINVAL;

    FOREACH_S(i, MAX_KEYBOARD_INTR_PIPES, MAX_KEYBOARD_INTR_PIPES, 0, 1)
    {
        struct sp_usbpdev_pipe *pipe = _kbd_intr_pipes[i];
        if (pipe)
        {
            status = _kbd_read_nonblock(pipe, out_1st_key_event, out_2nd_key_event);
            if (FAIL(status))
                return status;

            if (out_1st_key_event->action != LIBDRV_HID_KBD_NO_EVENT)
            {
                // A keyboard triggers an event
                return SUCCESS;
            }
        }
    }

    // No keyboard triggers an event, just return SUCCESS.
    return SUCCESS;
}

SAP_STATUS usb_kbd_read(struct kbd_key_event *out_key_event)
{
    SAP_STATUS status = SUCCESS;
    struct kbd_key_event result = {.action = LIBDRV_HID_KBD_NO_EVENT};
    struct kbd_key_event result2 = {.action = LIBDRV_HID_KBD_NO_EVENT};

    if (!out_key_event)
        return EINVAL;

    // Block if there is no kbd input read.
    while (PASS(status) && (result.action == LIBDRV_HID_KBD_NO_EVENT))
    {
        status = usb_kbd_read_nonblock(&result, &result2);

        if (FAIL(status))
            return status;
    }

    // Got the keyboard input.
    *out_key_event = result;
    return SUCCESS;
}

static void _kbd_free_intrpipe(struct sp_usbpdev_pipe *pipe)
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

            _kbd_intr_pipes[idx] = NULL;
        }
    }
}

//! \brief Read keyboard event due to periodically pull and/or mouse interrupts
SAP_STATUS usb_kbd_handler(void)
{
    SAP_STATUS status = SUCCESS;
    struct kbd_key_event *event = NULL;
    struct kbd_key_event *event2 = NULL;

    event = smalloc(sizeof(struct kbd_key_event));
    event2 = smalloc(sizeof(struct kbd_key_event));
    if (!event || !event2)
    {
        SAP_ERR("No memory to queue the new USB keyboard event!\n");
        status = ENOMEM;
        goto err;
    }

    {
        status = usb_kbd_read_nonblock(event, event2);
        if (FAIL(status))
        {
            goto err;
        }

        if (event->action != LIBDRV_HID_KBD_NO_EVENT)
        {
            print_kbd_key_event("USB", event);

            // Call all listeners
            libdrv_hid_call_kbd_listeners(event);
        }

        if (event2->action != LIBDRV_HID_KBD_NO_EVENT)
        {
            print_kbd_key_event("USB", event2);

            // Call all listeners
            libdrv_hid_call_kbd_listeners(event2);
        }
    }

    // On success
    sfree(event, sizeof(struct kbd_key_event));
    sfree(event2, sizeof(struct kbd_key_event));
    return SUCCESS;

err:
    if (event)
        sfree(event, sizeof(struct kbd_key_event));
    if (event2)
        sfree(event2, sizeof(struct kbd_key_event));

    return status;
}
