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
 * Authors: Eric Li
 *          Miao Yu
 *          Virgil Gligor
 */

#include <sap_headers_libc.h>
#include <palddk.h>
#include <drv/hid/input_events.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "debug.h"
#include "ps2-kbd.h"

extern void libdrv_hid_kbd_event_enqueue(struct kbd_key_event* event);
extern void libdrv_hid_call_kbd_listeners(struct kbd_key_event* event);

#define KBSTATP         0x64    // kbd controller status port(I)
#define KBDATAP         0x60    // kbd data port(I)

#define NO              0

extern uint8_t inportb(uint16_t port);

static uint32_t libdrv_hid_shiftcode[256] =
{
  [0x1D] LIBDRV_HID_MOD_LEFT_CTRL,
  [0x2A] LIBDRV_HID_MOD_LEFT_SHIFT,
  [0x36] LIBDRV_HID_MOD_RIGHT_SHIFT,
  [0x38] LIBDRV_HID_MOD_LEFT_ALT,
  [0x9D] LIBDRV_HID_MOD_RIGHT_CTRL,
  [0xB8] LIBDRV_HID_MOD_RIGHT_ALT,
  [0xDB] LIBDRV_HID_MOD_LEFT_GUI,
  [0xDC] LIBDRV_HID_MOD_RIGHT_GUI
};

static uint32_t libdrv_hid_togglecode[256] =
{
  [0x3A] LIBDRV_HID_MOD_CAPS_LOCK,
  [0x45] LIBDRV_HID_MOD_NUM_LOCK,
  [0x46] LIBDRV_HID_MOD_SCROLL_LOCK
};

static unsigned char libdrv_hid_map[256] =
{
  NO,                           // 0x00
  LIBDRV_HID_KEY_ESC,
  LIBDRV_HID_KEY_1,
  LIBDRV_HID_KEY_2,
  LIBDRV_HID_KEY_3,
  LIBDRV_HID_KEY_4,
  LIBDRV_HID_KEY_5,
  LIBDRV_HID_KEY_6,
  LIBDRV_HID_KEY_7,
  LIBDRV_HID_KEY_8,
  LIBDRV_HID_KEY_9,
  LIBDRV_HID_KEY_0,
  LIBDRV_HID_KEY_MINUS,
  LIBDRV_HID_KEY_EQUAL,
  LIBDRV_HID_KEY_BACKSPACE,
  LIBDRV_HID_KEY_TAB,
  LIBDRV_HID_KEY_Q,             // 0x10
  LIBDRV_HID_KEY_W,
  LIBDRV_HID_KEY_E,
  LIBDRV_HID_KEY_R,
  LIBDRV_HID_KEY_T,
  LIBDRV_HID_KEY_Y,
  LIBDRV_HID_KEY_U,
  LIBDRV_HID_KEY_I,
  LIBDRV_HID_KEY_O,
  LIBDRV_HID_KEY_P,
  LIBDRV_HID_KEY_LEFT_SQUARE,
  LIBDRV_HID_KEY_RIGHT_SQUARE,
  LIBDRV_HID_KEY_ENTER,
  LIBDRV_HID_KEY_LEFT_CTRL,
  LIBDRV_HID_KEY_A,
  LIBDRV_HID_KEY_S,
  LIBDRV_HID_KEY_D,             // 0x20
  LIBDRV_HID_KEY_F,
  LIBDRV_HID_KEY_G,
  LIBDRV_HID_KEY_H,
  LIBDRV_HID_KEY_J,
  LIBDRV_HID_KEY_K,
  LIBDRV_HID_KEY_L,
  LIBDRV_HID_KEY_SEMICOLON,
  LIBDRV_HID_KEY_QUOTE,
  LIBDRV_HID_KEY_BACKTICK,
  LIBDRV_HID_KEY_LEFT_SHIFT,
  LIBDRV_HID_KEY_BACKSLASH,
  LIBDRV_HID_KEY_Z,
  LIBDRV_HID_KEY_X,
  LIBDRV_HID_KEY_C,
  LIBDRV_HID_KEY_V,
  LIBDRV_HID_KEY_B,             // 0x30
  LIBDRV_HID_KEY_N,
  LIBDRV_HID_KEY_M,
  LIBDRV_HID_KEY_COMMA,
  LIBDRV_HID_KEY_PERIOD,
  LIBDRV_HID_KEY_SLASH,
  LIBDRV_HID_KEY_RIGHT_SHIFT,
  LIBDRV_HID_KEYPAD_ASTERIX,
  LIBDRV_HID_KEY_LEFT_ALT,
  LIBDRV_HID_KEY_SPACE,
  LIBDRV_HID_KEY_CAPS_LOCK,
  LIBDRV_HID_KEY_F1,
  LIBDRV_HID_KEY_F2,
  LIBDRV_HID_KEY_F3,
  LIBDRV_HID_KEY_F4,
  LIBDRV_HID_KEY_F5,
  LIBDRV_HID_KEY_F6,            // 0x40
  LIBDRV_HID_KEY_F7,
  LIBDRV_HID_KEY_F8,
  LIBDRV_HID_KEY_F9,
  LIBDRV_HID_KEY_F10,
  LIBDRV_HID_KEY_NUM_LOCK,
  LIBDRV_HID_KEY_SCROLL_LOCK,
  LIBDRV_HID_KEYPAD_7,
  LIBDRV_HID_KEYPAD_8,
  LIBDRV_HID_KEYPAD_9,
  LIBDRV_HID_KEYPAD_MINUS,
  LIBDRV_HID_KEYPAD_4,
  LIBDRV_HID_KEYPAD_5,
  LIBDRV_HID_KEYPAD_6,
  LIBDRV_HID_KEYPAD_PLUS,
  LIBDRV_HID_KEYPAD_1,
  LIBDRV_HID_KEYPAD_2,          // 0x50
  LIBDRV_HID_KEYPAD_3,
  LIBDRV_HID_KEYPAD_0,
  LIBDRV_HID_KEYPAD_PERIOD,
  NO,
  NO,
  NO,
  LIBDRV_HID_KEY_F11,
  LIBDRV_HID_KEY_F12,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0x60
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0x70
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0x80
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0x90
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  LIBDRV_HID_KEYPAD_ENTER,
  LIBDRV_HID_KEY_RIGHT_CTRL,
  NO,
  NO,
  NO,                           // 0xa0
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0xb0
  NO,
  NO,
  NO,
  NO,
  LIBDRV_HID_KEYPAD_SLASH,
  NO,
  LIBDRV_HID_KEY_PRINTSCREEN,
  LIBDRV_HID_KEY_RIGHT_ALT,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0xc0
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  LIBDRV_HID_KEY_HOME,
  LIBDRV_HID_KEY_UP,
  LIBDRV_HID_KEY_PAGE_UP,
  NO,
  LIBDRV_HID_KEY_LEFT,
  NO,
  LIBDRV_HID_KEY_RIGHT,
  NO,
  LIBDRV_HID_KEY_END,
  LIBDRV_HID_KEY_DOWN,          // 0xd0
  LIBDRV_HID_KEY_PAGE_DOWN,
  LIBDRV_HID_KEY_INSERT,
  LIBDRV_HID_KEY_DELETE,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  LIBDRV_HID_KEY_LEFT_GUI,
  LIBDRV_HID_KEY_RIGHT_GUI,
  LIBDRV_HID_KEY_MENU,
  NO,
  NO,
  NO,                           // 0xe0
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,                           // 0xf0
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO,
  NO
};

static bool e0_esc = false;
static bool e1_pause_seq1 = false;
static bool e1_pause_seq2 = false;
static bool e1_pause_seq3 = false;
static bool e1_pause_seq4 = false;
static bool e1_pause_seq5 = false;

// See https://wiki.osdev.org/PS/2_Keyboard
bool kbd_get_key_event_readbuf(struct kbd_key_event *event)
{
    // Whether previous data read is 0xe0
    static uint32_t hid_shift;

    bool pressed;
    uint32_t data;

    data = inportb(PS2_DATA);

    // Test "0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5" for Pause/Break
    if((data == 0xC5) && e1_pause_seq5)
    {
        event->action = LIBDRV_HID_KBD_KEY_PRESSED;
        event->modifier = hid_shift;
        event->key = LIBDRV_HID_KEY_PAUSE;
        return true;
    }

    if((data == 0x9D) && e1_pause_seq4)
    {
        e1_pause_seq5 = true;
        return false;
    }
    else
        e1_pause_seq5 = false;

    if((data == 0xE1) && e1_pause_seq3)
    {
        e1_pause_seq4 = true;
        return false;
    }
    else
        e1_pause_seq4 = false;

    if((data == 0x45) && e1_pause_seq2)
    {
        e1_pause_seq3 = true;
        return false;
    }
    else
        e1_pause_seq3 = false;

    if((data == 0x1D) && e1_pause_seq1)
    {
        e1_pause_seq2 = true;
        return false;
    }
    else
        e1_pause_seq2 = false;

    if(data == 0xE1)
    {
        e1_pause_seq1 = true;
        return false;
    }
    else
        e1_pause_seq1 = false;


    // Handle 0xE0 extension
    if(data == 0xE0){
        e0_esc = true;
        return false;
    }

    // Skip "0xE0, 0x2A" and "0xE0, 0xAA"
    if(e0_esc)
    {
        if(data == 0x2A || data == 0xAA)
        {
            e0_esc = false;
            return false;
        }
    }

    // Parse keypress
    if (data & 0x80) {
        // Key released
        pressed = false;
    } else {
        // Key pressed
        pressed = true;
    }

    data &= 0x7F;
    if (e0_esc) {
        data |= 0x80;
        e0_esc = false;
    }

    if (pressed) {
        hid_shift |= libdrv_hid_shiftcode[data];
        hid_shift ^= libdrv_hid_togglecode[data];
        event->action = LIBDRV_HID_KBD_KEY_PRESSED;
    } else {
        hid_shift &= ~(libdrv_hid_shiftcode[data]);
        event->action = LIBDRV_HID_KBD_KEY_RELEASED;
    }

    event->modifier = hid_shift;
    event->key = libdrv_hid_map[data];
    return true;
}

/*
 * Communicate with keyboard and see whether an event is generated.
 * Return false when no event is generated.
 * Return true when event is generated.
 */
static bool kbd_get_key_event(struct kbd_key_event *event)
{
    uint32_t st;

    st = inportb(PS2_STATUS);
    if((st & PS2_STS_OBF) == 0)
        return false;

    return kbd_get_key_event_readbuf(event);
}

void kbd_handler(void)
{
    // struct kbd_key_event event = {0};
    struct kbd_key_event* event = NULL;

    event = smalloc(sizeof(struct kbd_key_event));
    if(!event)
    {
        SAP_ERR("No memory to queue the new PS2 keyboard event!\n");
        goto err;
    }

    if (!kbd_get_key_event(event))
        goto err;

    print_kbd_key_event("PS2", event);

    // Call all listeners
    libdrv_hid_call_kbd_listeners(event);

    // // Enqueue the mouse event
    // libdrv_hid_kbd_event_enqueue(event);

    // On success
	return;


err:
    if(event)
		sfree(event, sizeof(struct kbd_key_event));
}

void kbd_handler_poll(uint32_t ps2_status)
{
    // struct kbd_key_event event = {0};
    struct kbd_key_event* event = NULL;

    event = smalloc(sizeof(struct kbd_key_event));
    if(!event)
    {
        SAP_ERR("Polling keyboard error: No memory to queue the new PS2 keyboard event!\n");
        goto err;
    }

    if (!kbd_get_key_event_readbuf(event)) {
        goto err;
    }

    print_kbd_key_event("PS2", event);

    // sos_sys_test(0);

    // Call all listeners
    libdrv_hid_call_kbd_listeners(event);

    // // Enqueue the mouse event
    // libdrv_hid_kbd_event_enqueue(event);

    // On success
	return;


err:
    if(event)
		sfree(event, sizeof(struct kbd_key_event));
}