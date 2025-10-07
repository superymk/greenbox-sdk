// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Eric Li (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers_libc.h>
#include <palddk.h>
#include <drv/hid/input_events.h>
#include <stdio.h>

#include "ps2-mouse.h"
#include "debug.h"

typedef enum
{
    PS2_AUX_INVALID = 0,
    PS2_AUX_MOUSE = 1,
} PS2_AUX_TYPE;

extern void libdrv_hid_mouse_event_enqueue(struct mouse_event *event);
extern void libdrv_hid_call_mouse_listeners(struct mouse_event *event);
static SAP_STATUS ps2_mouse_translate_to_libhid_event(ps2_mouse_t *old_state, ps2_mouse_t *new_state);
static SAP_STATUS ps2_touchpad_translate_to_libhid_event(ps2_touchpad_t *old_state, ps2_touchpad_t *new_state);

static void ps2_mouse_handle_packet(void);
static void ps2_touchpad_handle_packet(PS2_AUX_TYPE ps2_aux_type);

static uint32_t current_byte = 0;
static uint32_t bytes_per_packet = 3;
static uint8_t packet[10] = {0};

// static uint32_t device;
static ps2_mouse_t ps2_mouse_state;
static ps2_touchpad_t ps2_touchpad_state;

static PS2_AUX_TYPE cur_packet_type = PS2_AUX_INVALID;

static PS2_DEV_TYPE mouse_type = PS2_DEVICE_UNKNOWN;

static uint8_t ps2_os_mode_byte = 0;

// /* Initializes a mouse plugged in controller `dev`.
//  * Tries to enable as many of its features as possible.
//  */
// void init_mouse(uint32_t dev)
// {
//     device = dev;

//     // irq_register_handler(IRQ12, mouse_handle_interrupt);

//     // Enable features
//     mouse_enable_scroll_wheel();
//     mouse_enable_five_buttons();

//     // Set mouse parameters
//     mouse_set_sample_rate(80);
//     mouse_set_resolution(0x00); // One unit per mm
//     mouse_set_scaling(false);   // Disable acceleration

//     // Start receiving IRQ12s
//     ps2_write_device(device, PS2_DEV_ENABLE_SCAN);
//     ps2_expect_ack();
// }

/// @brief Return true iff the bit <bit> is clear in the value <val>
static inline bool test_bit_set(uint32_t num, int bit)
{
    return num & (1 << bit) ? true : false;
}

/// @brief Return true iff the bit <bit> is clear in the value <val>
static inline bool test_bit_clear(uint32_t num, int bit)
{
    return num & (1 << bit) ? false : true;
}

void mouse_handle_polling(uint32_t ps2_status)
{
    uint8_t byte = 0;
    bool status = true;

    status = ps2_read(PS2_DATA, &byte);
    if (!status)
        return;

    // Try to stay synchronized by discarding obviously out of place bytes
    if (mouse_type == PS2_MOUSE_SCROLL_WHEEL)
    {
        // For PS2 mouse, Bit 3 of Byte 0 is always set
        // This is a PS2 mouse, so we expect 3 bytes
        bytes_per_packet = 4;

        // Resync if needed
        if (current_byte == 0 && !(test_bit_set(byte, PS2_MOUSE_BYTE0_BIT3_MUST_SET)))
        {
            // Re-sync
            return;
        }
    }
    else if ((mouse_type == PS2_TOUCHPAD) || (mouse_type == PS2_TOUCHPAD_THINKPAD))
    {
        bytes_per_packet = 6;

        // SAP_ERR("[Superymk] <mouse_handle_polling> current_byte:%u, byte[0]:0x%02X\n", current_byte, byte);

        // Resync if needed
        if (current_byte == 0 &&
            !(test_bit_set(byte, PS2_TOUCHPAD_BYTE0_BIT7_MUST_SET) &&
              test_bit_clear(byte, PS2_TOUCHPAD_BYTE0_BIT6_MUST_CLEAR) &&
              test_bit_clear(byte, PS2_TOUCHPAD_BYTE0_BIT3_MUST_CLEAR)))
        {
            // Re-sync
            return;
        }
    }

    // if (current_byte == 0 && test_bit_set(byte, 3))
    // {
    //     // For PS2 mouse, Bit 3 of Byte 0 is always set
    //     // This is a PS2 mouse, so we expect 3 bytes
    //     cur_packet_type = PS2_AUX_MOUSE;
    //     bytes_per_packet = 4;
    // }
    // else if (current_byte == 0 && test_bit_set(byte, 7) && test_bit_clear(byte, 3))
    // {
    //     // For PS2 touchpad, Bit 7 of Byte 0 is always set and Bit 3 of Byte 0 is always clear
    //     // This is a PS2 touchpad (in absolute mode), so we expect 6 bytes
    //     if(_is_thinkpad_touchpad)
    //     {
    //         cur_packet_type = PS2_AUX_TOUCHPAD_THINKPAD;
    //         bytes_per_packet = 6;
    //     }
    //     else
    //     {
    //         cur_packet_type = PS2_AUX_TOUCHPAD;
    //         bytes_per_packet = 6;
    //     }
    // }
    // else if (current_byte == 0 && !(byte & PS2_MOUSE_ALWAYS_SET))
    // {
    //     // Re-sync
    //     cur_packet_type = PS2_AUX_INVALID;
    //     return;
    // }

    packet[current_byte] = byte;
    current_byte = (current_byte + 1) % bytes_per_packet;

    // We've received a full packet
    if (current_byte == 0)
    {
        if (mouse_type == PS2_MOUSE_SCROLL_WHEEL)
            ps2_mouse_handle_packet();
        else if ((mouse_type == PS2_TOUCHPAD) || (mouse_type == PS2_TOUCHPAD_THINKPAD))
            ps2_touchpad_handle_packet(cur_packet_type);
    }
}

/* Receives one byte from the mouse and packages it in `packet`. Once that
 * packet is full, calls `ps2_mouse_handle_packet`.
 */
void mouse_handle_interrupt(void)
{
    mouse_handle_polling(0);
}

bool ps2_mouse_states_equal(ps2_mouse_t *a, ps2_mouse_t *b)
{
    return a->x == b->x && a->y == b->y &&
           a->left_pressed == b->left_pressed &&
           a->middle_pressed == b->middle_pressed &&
           a->right_pressed == b->right_pressed;
}

bool ps2_touchpad_states_equal(ps2_touchpad_t *a, ps2_touchpad_t *b)
{
    return a->x == b->x && a->y == b->y &&
           a->left_pressed == b->left_pressed &&
           a->middle_pressed == b->middle_pressed &&
           a->right_pressed == b->right_pressed;
}

static SAP_STATUS send_tp_arg(uint32_t dev_num, uint8_t arg)
{
    SAP_STATUS status = SUCCESS;
    bool ret = false;
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        ret = ps2_write_device(dev_num, 0xE8);
        if (!ret)
            goto err;
        status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
        if (FAIL(status))
            goto err;

        ret = ps2_write_device(dev_num, (arg >> (6 - 2 * i)) & 3);
        if (!ret)
            goto err;
        status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
        if (FAIL(status))
            goto err;
    }

    return SUCCESS;

err:
    return EINTERNAL;
}

static void ps2_mouse_handle_packet(void)
{
    SAP_STATUS status = SUCCESS;
    ps2_mouse_t ps2_mouse_old_state = ps2_mouse_state;

    uint8_t flags = packet[0];
    int32_t delta_x = (int32_t)packet[1];
    int32_t delta_y = (int32_t)packet[2];
    uint8_t extra = 0;
    int8_t wheel = 0;

    // SAP_ERR("[Superymk] <ps2_mouse_handle_packet> packet[0]:0x%08X, packet[1]:0x%08X, packet[2]:0x%08X, packet[3]:0x%08X, bytes_per_packet:%u\n",
    //             packet[0], packet[1], packet[2], packet[3], bytes_per_packet);

    // Superior mice send four bytes
    if (bytes_per_packet == 4)
    {
        extra = packet[3];

        // if (extra & MOUSE_UNUSED_A || extra & MOUSE_UNUSED_B)
        // {
        //     return; // Unused bits are set: beware
        // }
    }

    // Packets with X or Y overflow are probably garbage
    if (flags & MOUSE_X_OVERFLOW || flags & MOUSE_Y_OVERFLOW)
    {
        return;
    }

    // Two's complement by hand
    if (flags & MOUSE_X_NEG)
    {
        delta_x |= 0xFFFFFF00;
    }

    if (flags & MOUSE_Y_NEG)
    {
        delta_y |= 0xFFFFFF00;
    }

    // Wheel
    {
        // see https://stackoverflow.com/questions/48148071/will-a-usb-mouse-through-a-ps-2-converter-support-mouse-wheel-scrolling
        wheel = extra & 0x7;
        if (extra & MOUSE_EXTRA_BIT_SIGN)
            wheel = -(8 - wheel);
    }

    ps2_mouse_state.left_pressed = flags & MOUSE_LEFT;
    ps2_mouse_state.right_pressed = flags & MOUSE_RIGHT;
    ps2_mouse_state.middle_pressed = flags & MOUSE_MIDDLE;

    ps2_mouse_state.x += delta_x;
    ps2_mouse_state.y -= delta_y; // Point the y-axis downward

    ps2_mouse_state.delta_x = delta_x;
    ps2_mouse_state.delta_y = delta_y;

    ps2_mouse_state.wheel = wheel;

    // if (!ps2_mouse_states_equal(&ps2_mouse_old_state, &ps2_mouse_state))
    {
        status = ps2_mouse_translate_to_libhid_event(&ps2_mouse_old_state, &ps2_mouse_state);
        (void)status;
    }
}

// See Section 3.2.1 of "Synaptics PS/2 TouchPad Interfacing Guide, Rev B"
#define PS2_TOUCHPAD_CALC_XPOS(x_b2, x_b4, x_b5) (((x_b4) << 12) | ((x_b2) << 8) | (x_b5))
#define PS2_TOUCHPAD_CALC_YPOS(y_b2, y_b4, y_b6) (((y_b4) << 12) | ((y_b2) << 8) | (y_b6))

static void ps2_touchpad_handle_packet(PS2_AUX_TYPE ps2_aux_type)
{
    SAP_STATUS status = SUCCESS;
    ps2_touchpad_t ps2_touchpad_old_state = ps2_touchpad_state;
    uint32_t x_pos = 0;
    uint32_t y_pos = 0;

    SAP_ERR("[Superymk] <ps2_touchpad_handle_packet> packet[0]:0x%08X, packet[1]:0x%08X, packet[2]:0x%08X, packet[3]:0x%08X, packet[4]:0x%08X, packet[5]:0x%08X, bytes_per_packet:%u\n",
            packet[0], packet[1], packet[2], packet[3], packet[4], packet[5], bytes_per_packet);

    // if(ps2_aux_type == PS2_AUX_TOUCHPAD_THINKPAD)
    // {
    //     // Add filter for Thinkpad touchpad
    //     if((packet[0] == 0x80) &&
    //         (packet[1] == 0) &&
    //         (packet[2] == 0) &&
    //         (packet[3] == 0xC0) &&
    //         (packet[4] == 0) &&
    //         (packet[5] == 0)
    //     )
    //     {
    //         // Drop the dummy packet
    //         return;
    //     }
    // }

    if (mouse_type == PS2_TOUCHPAD)
    {
        x_pos = PS2_TOUCHPAD_CALC_XPOS(packet[1] & 0xF, (packet[3] >> 4) & 0x1, packet[4]);
        y_pos = PS2_TOUCHPAD_CALC_YPOS((packet[1] >> 4) & 0xF, (packet[3] >> 5) & 0x1, packet[5]);
        ps2_touchpad_state.x = x_pos;
        ps2_touchpad_state.y = y_pos;
    }
    else if (mouse_type == PS2_TOUCHPAD_THINKPAD)
    {
        if (packet[0] & 0x10)
        {
            x_pos = PS2_TOUCHPAD_CALC_XPOS(packet[1] & 0xF, (packet[3] >> 4) & 0x1, packet[4]);
            y_pos = PS2_TOUCHPAD_CALC_YPOS((packet[1] >> 4) & 0xF, (packet[3] >> 5) & 0x1, packet[5]);
            ps2_touchpad_state.x = x_pos;
            ps2_touchpad_state.y = y_pos;
        }
    }

    if (mouse_type == PS2_TOUCHPAD)
    {
        ps2_touchpad_state.left_pressed = packet[0] & PS2_TOUCHPAD_BYTE0_LEFT;
        ps2_touchpad_state.right_pressed = packet[0] & PS2_TOUCHPAD_BYTE0_RIGHT;
    }
    else if (mouse_type == PS2_TOUCHPAD_THINKPAD)
    {
        // Check the buttons above the touchpad
        if (packet[0] & 0x4)
        {
            ps2_touchpad_state.left_pressed = packet[1] & PS2_TOUCHPAD_THINKPAD_BYTE1_LEFT;
            ps2_touchpad_state.right_pressed = packet[1] & PS2_TOUCHPAD_THINKPAD_BYTE1_RIGHT;
            ps2_touchpad_state.middle_pressed = packet[1] & PS2_TOUCHPAD_THINKPAD_BYTE1_MIDDLE;
        }
        else
        {
            // Check the buttons in the touchpad
            {
                bool btn_pressed = packet[3] & PS2_TOUCHPAD_THINKPAD_BYTE3_BTN_PRESS;

                if (x_pos < 2000)
                    ps2_touchpad_state.left_pressed = btn_pressed;
                else if (x_pos < 4000)
                    ps2_touchpad_state.middle_pressed = btn_pressed;
                else
                    ps2_touchpad_state.right_pressed = btn_pressed;
            }
        }
    }

    // SAP_ERR("[Superymk] <ps2_touchpad_handle_packet> left_pressed:%u, right_pressed:%u\n", ps2_touchpad_state.left_pressed, ps2_touchpad_state.right_pressed);

    if (!ps2_touchpad_states_equal(&ps2_touchpad_old_state, &ps2_touchpad_state))
    {
        status = ps2_touchpad_translate_to_libhid_event(&ps2_touchpad_old_state, &ps2_touchpad_state);
        (void)status;
    }
}

static SAP_STATUS ps2_mouse_translate_to_libhid_event(ps2_mouse_t *old_state, ps2_mouse_t *new_state)
{
    // struct mouse_event event = {0};
    struct mouse_event *event = NULL;
    bool wheel = false;
    static uint32_t last_wheel = 0;

    // Check: <state> must be valid
    if (!old_state || !new_state)
        return EINVAL;

    event = smalloc(sizeof(struct mouse_event));
    if (!event)
    {
        SAP_ERR("No memory to queue the new PS2 mouse event!\n");
        return ENOMEM;
    }

    // Mouse key presses
    if (!old_state->left_pressed && new_state->left_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button |= LIBDRV_HID_BTN_LEFT;
    }
    else if (old_state->left_pressed && !new_state->left_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_RELEASED;
        event->button |= LIBDRV_HID_BTN_LEFT;
    }

    if (!old_state->right_pressed && new_state->right_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button |= LIBDRV_HID_BTN_RIGHT;
    }
    else if (old_state->right_pressed && !new_state->right_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_RELEASED;
        event->button |= LIBDRV_HID_BTN_RIGHT;
    }

    if (!old_state->middle_pressed && new_state->middle_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button |= LIBDRV_HID_BTN_MIDDLE;
    }
    else if (old_state->middle_pressed && !new_state->middle_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_RELEASED;
        event->button |= LIBDRV_HID_BTN_MIDDLE;
    }

    // Wheel
    if (new_state->wheel != 0)
    {
        if (new_state->wheel < 0)
        {
            event->wheel = last_wheel = LIBDRV_HID_WHEEL_UP;
            event->wheel_diff = 0 - new_state->wheel;
            wheel = true;
        }
        else if (new_state->wheel > 0)
        {
            event->wheel = last_wheel = LIBDRV_HID_WHEEL_DOWN;
            event->wheel_diff = new_state->wheel;
            wheel = true;
        }

        if (wheel)
        {
            event->button |= LIBDRV_HID_BTN_WHEEL;
            event->action = LIBDRV_HID_MOUSE_BTN_PRESSED;
        }
    }
    else
    {
        // Wheel release
        if (old_state->wheel != 0)
        {
            event->wheel = last_wheel;
            last_wheel = 0;

            event->button |= LIBDRV_HID_BTN_WHEEL;
            event->action = LIBDRV_HID_MOUSE_BTN_RELEASED;
        }
    }

    // Mouse move
    if (!event->action)
    {
        if (new_state->delta_x || new_state->delta_y)
        {
            event->action = LIBDRV_HID_MOUSE_MOVED;
            event->delta_x = new_state->delta_x;
            event->delta_y = -new_state->delta_y;
        }
        else
        {
            // If event->action, <delta_x>, and <delta_y> are 0, the mouse event must be a spurious one
            event->action = LIBDRV_HID_MOUSE_NO_EVENT;
        }
    }

    print_mouse_event("PS2", event);

    // Call all listeners
    libdrv_hid_call_mouse_listeners(event);

    // // Enqueue the mouse event
    // libdrv_hid_mouse_event_enqueue(event);

    return SUCCESS;
}

#define TOUCHPAD_ABSOLUTE_IGNORE_X_Y_THRESHOLD (100)

static SAP_STATUS ps2_touchpad_translate_to_libhid_event(ps2_touchpad_t *old_state, ps2_touchpad_t *new_state)
{
    // struct mouse_event event = {0};
    struct mouse_event *event = NULL;

    // Check: <state> must be valid
    if (!old_state || !new_state)
        return EINVAL;

    event = smalloc(sizeof(struct mouse_event));
    if (!event)
    {
        SAP_ERR("No memory to queue the new PS2 touchpad event!\n");
        return ENOMEM;
    }

    // Mouse key presses
    if (!old_state->left_pressed && new_state->left_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button |= LIBDRV_HID_BTN_LEFT;
    }
    else if (old_state->left_pressed && !new_state->left_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_RELEASED;
        event->button |= LIBDRV_HID_BTN_LEFT;
    }

    if (!old_state->right_pressed && new_state->right_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button |= LIBDRV_HID_BTN_RIGHT;
    }
    else if (old_state->right_pressed && !new_state->right_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_RELEASED;
        event->button |= LIBDRV_HID_BTN_RIGHT;
    }

    if (!old_state->middle_pressed && new_state->middle_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_PRESSED;
        event->button |= LIBDRV_HID_BTN_MIDDLE;
    }
    else if (old_state->middle_pressed && !new_state->middle_pressed)
    {
        event->action |= LIBDRV_HID_MOUSE_BTN_RELEASED;
        event->button |= LIBDRV_HID_BTN_MIDDLE;
    }

    // Mouse move
    if (!event->action)
    {
        int delta_x = new_state->x - old_state->x;
        int delta_y = -(new_state->y - old_state->y);

        if (delta_x || delta_y)
        {
            // Touchpads are in absolute position mode. So we ignore user's new touch position if they are too far away
            if ((ABS(delta_x) <= TOUCHPAD_ABSOLUTE_IGNORE_X_Y_THRESHOLD) && (ABS(delta_y) <= TOUCHPAD_ABSOLUTE_IGNORE_X_Y_THRESHOLD))
            {
                event->action = LIBDRV_HID_MOUSE_MOVED;
                event->delta_x = delta_x;
                event->delta_y = delta_y;
            }
            else
            {
                // The new position is too far away. Ignore the event.  
                event->action = LIBDRV_HID_MOUSE_NO_EVENT;
            }
        }
        else
        {
            // If event->action, <delta_x>, and <delta_y> are 0, the mouse event must be a spurious one
            event->action = LIBDRV_HID_MOUSE_NO_EVENT;
        }
    }

    print_mouse_event("PS2-Touchpad", event);

    // Call all listeners
    libdrv_hid_call_mouse_listeners(event);

    // // Enqueue the mouse event
    // libdrv_hid_mouse_event_enqueue(event);

    return SUCCESS;
}

static SAP_STATUS _ps2_touchpad_thinkpad_read_os_mode(uint32_t dev_num, uint8_t* out_os_mode)
{
    SAP_STATUS status = SUCCESS;
    uint8_t info_byte0 = 0, info_byte1 = 0, info_byte2 = 0;

    // Send command "Read TouchPad Modes", see Section 4.4 of "Synaptics PS/2 TouchPad Interfacing Guide, Rev B"
    status = send_tp_arg(dev_num, 0x01);
    if (FAIL(status))
    {
        SAP_ERR("Failed to send command to PS2 touchpad! cmd: 0x%02X, status:%lu\n", 0x00, status);
        return EINTERNAL;
    }

    ps2_write_device(dev_num, 0xE9);
    status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if (FAIL(status))
        return EINTERNAL;

    // Read response
    status = ps2_read(PS2_DATA, &info_byte0);
    if (!status)
        return EINTERNAL;

    status = ps2_read(PS2_DATA, &info_byte1);
    if (!status)
        return EINTERNAL;

    status = ps2_read(PS2_DATA, &info_byte2);
    if (!status)
        return EINTERNAL;

    SAP_INFO("[Superymk] <_ps2_touchpad_thinkpad_read_os_mode> info_byte2:0x%02X\n", info_byte2);

    // On success
    if(out_os_mode)
        *out_os_mode = info_byte2;
    return SUCCESS;
}

static SAP_STATUS _ps2_touchpad_thinkpad_setmode(uint32_t dev_num, uint8_t mode)
{
    SAP_STATUS status = SUCCESS;
    bool ret = false;

    // Disables scanning
    ps2_write_device(dev_num, PS2_DEV_DISABLE_SCAN);
    status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if (FAIL(status))
        return EINTERNAL;

    // Set the mode byte
    status = send_tp_arg(dev_num, mode);
    if (FAIL(status))
        return EINTERNAL;

    ret = ps2_write_device(dev_num, 0xF3);
    if (!ret)
        return EINTERNAL;
    status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if (FAIL(status))
        return EINTERNAL;

    ret = ps2_write_device(dev_num, 0x14);
    if (!ret)
        return EINTERNAL;
    status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if (FAIL(status))
        return EINTERNAL;

    // Enables scanning
    ps2_write_device(dev_num, PS2_DEV_ENABLE_SCAN);
    status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if (FAIL(status))
        return EINTERNAL;

    SAP_INFO("ThinkPad Touchpad reset mode succeed\n");

    // On success
    return SUCCESS;
}

static SAP_STATUS _ps2_touchpad_identify(uint32_t dev_num)
{
    SAP_STATUS status = SUCCESS;
    uint8_t info_byte0 = 0, info_byte1 = 0, info_byte2 = 0;

    // Send command "Identify TouchPad", see Section 4.4 of "Synaptics PS/2 TouchPad Interfacing Guide, Rev B"
    status = send_tp_arg(dev_num, 0x00);
    if (FAIL(status))
    {
        SAP_ERR("Failed to send command to PS2 touchpad! cmd: 0x%02X, status:%lu\n", 0x00, status);
        return EINTERNAL;
    }

    ps2_write_device(dev_num, 0xE9);
    status = ps2_expect_ack(PS2_ACK_TIMEOUT_MS);
    if (FAIL(status))
        return EINTERNAL;

    // Read response
    status = ps2_read(PS2_DATA, &info_byte0);
    if (!status)
        return EINTERNAL;

    status = ps2_read(PS2_DATA, &info_byte1);
    if (!status)
        return EINTERNAL;

    status = ps2_read(PS2_DATA, &info_byte2);
    if (!status)
        return EINTERNAL;

    // see Section 4.4 of "Synaptics PS/2 TouchPad Interfacing Guide, Rev B"
    if (info_byte1 == 0x47)
    {
        mouse_type = PS2_TOUCHPAD;
        SAP_INFO("PS2 Aux is Synaptics TouchPad (Major:0x%02X, Minor:0x%02X, ModelCode:0x%02X)\n", (info_byte2 & 0xF), info_byte0, (info_byte2 >> 4));

        if (info_byte0 == 0x02 && info_byte2 == 0x18)
        {
            mouse_type = PS2_TOUCHPAD_THINKPAD;
            SAP_INFO("\t Found Thinkpad Touchpad\n");
        }
    }

    // On success
    return SUCCESS;
}

SAP_STATUS ps2_mouse_init(void)
{
    SAP_STATUS status = SUCCESS;
    PS2_DEV_TYPE type = PS2_DEVICE_UNKNOWN;

    // Identify PS2 mouse type
    type = ps2_identify_device(1);
    if (type != PS2_DEVICE_UNKNOWN)
        mouse_type = type;

    // Identify touchpad type
    if (type == PS2_MOUSE)
    {
        status = _ps2_touchpad_identify(1);
        if (FAIL(status))
        {
            SAP_ERR("Failed to identify the PS2 touchpad! status:%lu\n", status);
            return EINTERNAL;
        }

        if (mouse_type == PS2_TOUCHPAD_THINKPAD)
        {
            // Backup OS mode
            status = _ps2_touchpad_thinkpad_read_os_mode(1, &ps2_os_mode_byte);
            if (FAIL(status))
            {
                SAP_ERR("Failed to read the OS mode of the PS2 ThinkPad touchpad! status:%lu\n", status);
                return EINTERNAL;
            }

            // Set thinkpad touchpad to 0xC1 (Absolute mode, high packet rate, W mode enabled), see Section 4.3 of 
            // "Synaptics PS/2 TouchPad Interfacing Guide, Rev B"
            status = _ps2_touchpad_thinkpad_setmode(1, 0xC1);
            if (FAIL(status))
            {
                SAP_ERR("Failed to set the mode of the PS2 ThinkPad touchpad! status:%lu\n", status);
                return EINTERNAL;
            }
        }
    }

    // On success
    return SUCCESS;
}

SAP_STATUS ps2_mouse_fini(void)
{
    SAP_STATUS status = SUCCESS;

    if (mouse_type == PS2_TOUCHPAD_THINKPAD)
    {
        status = _ps2_touchpad_thinkpad_setmode(1, ps2_os_mode_byte);
        if (FAIL(status))
        {
            SAP_ERR("Failed to restore the OS mode of the PS2 ThinkPad touchpad! status:%lu\n", status);
            return EINTERNAL;
        }
    }
    
    // On success
    return SUCCESS;
}

// void mouse_set_sample_rate(uint8_t rate)
// {
//     ps2_write_device(device, MOUSE_SET_SAMPLE);
//     ps2_expect_ack();
//     ps2_write_device(device, rate);
//     ps2_expect_ack();
// }

// void mouse_set_resolution(uint8_t level)
// {
//     ps2_write_device(device, MOUSE_SET_RESOLUTION);
//     ps2_expect_ack();
//     ps2_write_device(device, level);
//     ps2_expect_ack();
// }

// void mouse_set_scaling(bool enabled)
// {
//     uint8_t cmd = enabled ? MOUSE_ENABLE_SCALING : MOUSE_DISABLE_SCALING;

//     ps2_write_device(device, cmd);
//     ps2_expect_ack();
// }

// /* Uses a magic sequence to enable scroll wheel support.
//  */
// void mouse_enable_scroll_wheel()
// {
//     mouse_set_sample_rate(200);
//     mouse_set_sample_rate(100);
//     mouse_set_sample_rate(80);

//     uint32_t type = ps2_identify_device(device);

//     if (type == PS2_MOUSE_SCROLL_WHEEL)
//     {
//         bytes_per_packet = 4;
//         SAP_INFO("enabled scroll wheel\n");
//     }
//     else
//     {
//         SAP_ERR("unable to enable scroll wheel\n");
//     }
// }

// /* Uses a magic sequence to enable five buttons support.
//  */
// void mouse_enable_five_buttons()
// {
//     if (bytes_per_packet != 4)
//     {
//         return;
//     }

//     mouse_set_sample_rate(200);
//     mouse_set_sample_rate(200);
//     mouse_set_sample_rate(80);

//     uint32_t type = ps2_identify_device(device);

//     if (type != PS2_MOUSE_FIVE_BUTTONS)
//     {
//         SAP_INFO("mouse has fewer than five buttons\n");
//     }
//     else
//     {
//         SAP_INFO("five buttons enabled\n");
//     }
// }