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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <windows.h>

// #include <appddk.h>
#include <ipc/varbuf.h>
#include "secapp/app_sp_bridge_cmds.h"
#include "hid_bridge/hid_bridge_defs.h"
#include "hid_bridge/hid_ops.h"

static uint32_t libdrv_hid_to_hid_map[256] =
    {
        [LIBDRV_HID_KEY_ESC] VK_ESCAPE,
        [LIBDRV_HID_KEY_1] '1',
        [LIBDRV_HID_KEY_2] '2',
        [LIBDRV_HID_KEY_3] '3',
        [LIBDRV_HID_KEY_4] '4',
        [LIBDRV_HID_KEY_5] '5',
        [LIBDRV_HID_KEY_6] '6',
        [LIBDRV_HID_KEY_7] '7',
        [LIBDRV_HID_KEY_8] '8',
        [LIBDRV_HID_KEY_9] '9',
        [LIBDRV_HID_KEY_0] '0',
        [LIBDRV_HID_KEY_MINUS] VK_OEM_MINUS, // - (hyphen)
        [LIBDRV_HID_KEY_EQUAL] VK_OEM_PLUS,  // = (equal)
        [LIBDRV_HID_KEY_BACKSPACE] VK_BACK,
        [LIBDRV_HID_KEY_TAB] VK_TAB,
        [LIBDRV_HID_KEY_Q] 'Q',
        [LIBDRV_HID_KEY_W] 'W',
        [LIBDRV_HID_KEY_E] 'E',
        [LIBDRV_HID_KEY_R] 'R',
        [LIBDRV_HID_KEY_T] 'T',
        [LIBDRV_HID_KEY_Y] 'Y',
        [LIBDRV_HID_KEY_U] 'U',
        [LIBDRV_HID_KEY_I] 'I',
        [LIBDRV_HID_KEY_O] 'O',
        [LIBDRV_HID_KEY_P] 'P',
        [LIBDRV_HID_KEY_LEFT_SQUARE] VK_OEM_4,  // [ (left square bracket)
        [LIBDRV_HID_KEY_RIGHT_SQUARE] VK_OEM_6, // ] (right square bracket)
        [LIBDRV_HID_KEY_ENTER] VK_RETURN,
        [LIBDRV_HID_KEY_LEFT_CTRL] VK_LCONTROL,
        [LIBDRV_HID_KEY_A] 'A',
        [LIBDRV_HID_KEY_S] 'S',
        [LIBDRV_HID_KEY_D] 'D',
        [LIBDRV_HID_KEY_F] 'F',
        [LIBDRV_HID_KEY_G] 'G',
        [LIBDRV_HID_KEY_H] 'H',
        [LIBDRV_HID_KEY_J] 'J',
        [LIBDRV_HID_KEY_K] 'K',
        [LIBDRV_HID_KEY_L] 'L',
        [LIBDRV_HID_KEY_SEMICOLON] VK_OEM_1, // ; (semicolon)
        [LIBDRV_HID_KEY_QUOTE] VK_OEM_7,     // ' (apostrophe)
        [LIBDRV_HID_KEY_BACKTICK] VK_OEM_3,  // ` (backtick)
        [LIBDRV_HID_KEY_LEFT_SHIFT] VK_LSHIFT,
        [LIBDRV_HID_KEY_BACKSLASH] VK_OEM_5, // \ (backslash)
        [LIBDRV_HID_KEY_Z] 'Z',
        [LIBDRV_HID_KEY_X] 'X',
        [LIBDRV_HID_KEY_C] 'C',
        [LIBDRV_HID_KEY_V] 'V',
        [LIBDRV_HID_KEY_B] 'B',
        [LIBDRV_HID_KEY_N] 'N',
        [LIBDRV_HID_KEY_M] 'M',
        [LIBDRV_HID_KEY_COMMA] VK_OEM_COMMA,   // , (comma)
        [LIBDRV_HID_KEY_PERIOD] VK_OEM_PERIOD, // . (period)
        [LIBDRV_HID_KEY_SLASH] VK_OEM_2,       // / (slash)
        [LIBDRV_HID_KEY_RIGHT_SHIFT] VK_RSHIFT,
        [LIBDRV_HID_KEYPAD_ASTERIX] VK_MULTIPLY,
        [LIBDRV_HID_KEY_LEFT_ALT] VK_LMENU,
        [LIBDRV_HID_KEY_SPACE] VK_SPACE,
        [LIBDRV_HID_KEY_CAPS_LOCK] VK_CAPITAL,
        [LIBDRV_HID_KEY_F1] VK_F1,
        [LIBDRV_HID_KEY_F2] VK_F2,
        [LIBDRV_HID_KEY_F3] VK_F3,
        [LIBDRV_HID_KEY_F4] VK_F4,
        [LIBDRV_HID_KEY_F5] VK_F5,
        [LIBDRV_HID_KEY_F6] VK_F6,
        [LIBDRV_HID_KEY_F7] VK_F7,
        [LIBDRV_HID_KEY_F8] VK_F8,
        [LIBDRV_HID_KEY_F9] VK_F9,
        [LIBDRV_HID_KEY_F10] VK_F10,
        [LIBDRV_HID_KEY_NUM_LOCK] VK_NUMLOCK,
        [LIBDRV_HID_KEY_SCROLL_LOCK] VK_SCROLL,
        [LIBDRV_HID_KEYPAD_7] VK_NUMPAD7,
        [LIBDRV_HID_KEYPAD_8] VK_NUMPAD8,
        [LIBDRV_HID_KEYPAD_9] VK_NUMPAD9,
        [LIBDRV_HID_KEYPAD_MINUS] VK_SUBTRACT,
        [LIBDRV_HID_KEYPAD_4] VK_NUMPAD4,
        [LIBDRV_HID_KEYPAD_5] VK_NUMPAD5,
        [LIBDRV_HID_KEYPAD_6] VK_NUMPAD6,
        [LIBDRV_HID_KEYPAD_PLUS] VK_ADD,
        [LIBDRV_HID_KEYPAD_1] VK_NUMPAD1,
        [LIBDRV_HID_KEYPAD_2] VK_NUMPAD2,
        [LIBDRV_HID_KEYPAD_3] VK_NUMPAD3,
        [LIBDRV_HID_KEYPAD_0] VK_NUMPAD0,
        [LIBDRV_HID_KEYPAD_PERIOD] VK_DECIMAL,
        [LIBDRV_HID_KEY_F11] VK_F11,
        [LIBDRV_HID_KEY_F12] VK_F12,
        [LIBDRV_HID_KEY_PRINTSCREEN] VK_SNAPSHOT,
        [LIBDRV_HID_KEYPAD_ENTER] VK_RETURN,
        [LIBDRV_HID_KEY_RIGHT_CTRL] VK_RCONTROL,
        [LIBDRV_HID_KEYPAD_SLASH] VK_DIVIDE,
        [LIBDRV_HID_KEY_RIGHT_ALT] VK_RMENU,
        [LIBDRV_HID_KEY_HOME] VK_HOME,
        [LIBDRV_HID_KEY_UP] VK_UP,
        [LIBDRV_HID_KEY_PAGE_UP] VK_PRIOR,
        [LIBDRV_HID_KEY_LEFT] VK_LEFT,
        [LIBDRV_HID_KEY_RIGHT] VK_RIGHT,
        [LIBDRV_HID_KEY_END] VK_END,
        [LIBDRV_HID_KEY_DOWN] VK_DOWN,
        [LIBDRV_HID_KEY_PAGE_DOWN] VK_NEXT,
        [LIBDRV_HID_KEY_INSERT] VK_INSERT,
        [LIBDRV_HID_KEY_DELETE] VK_DELETE,
        [LIBDRV_HID_KEY_LEFT_GUI] VK_LWIN,
        [LIBDRV_HID_KEY_RIGHT_GUI] VK_RWIN};

/********* Public Functions *********/
bool os_hid_bridge_init(void)
{
    // On success
    HID_BRIDGE_INFO("Init Bridge-Virtual Keyboard and Mouse Success\n");
    return true;
}

void os_hid_bridge_fini(void)
{
    return;
}

static void hid_bridge_kbd_press(int code)
{
    uint32_t result = 0;
    INPUT in[1] = {};
    ZeroMemory(in, sizeof(in));

    in[0].type = INPUT_KEYBOARD;
    in[0].ki.wVk = (WORD)code; // VK_LWIN

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send keyboard press error!\n");
}

static void hid_bridge_kbd_release(int code)
{
    uint32_t result = 0;
    INPUT in[1] = {};
    ZeroMemory(in, sizeof(in));

    in[0].type = INPUT_KEYBOARD;
    in[0].ki.wVk = (WORD)code;
    in[0].ki.dwFlags = KEYEVENTF_KEYUP;

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send keyboard release error!\n");
}

static void hid_bridge_mouse_press(int code)
{
    uint32_t result = 0;
    INPUT in[1] = {};
    DWORD dwFlags;

    ZeroMemory(in, sizeof(in));

    if (code == LIBDRV_HID_BTN_LEFT)
        dwFlags = MOUSEEVENTF_LEFTDOWN;
    else if (code == LIBDRV_HID_BTN_RIGHT)
        dwFlags = MOUSEEVENTF_RIGHTDOWN;
    else if (code == LIBDRV_HID_BTN_MIDDLE)
        dwFlags = MOUSEEVENTF_MIDDLEDOWN;

    in[0].type = INPUT_MOUSE;
    in[0].mi.dwFlags = dwFlags;

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send mouse press error!\n");
}

static void hid_bridge_mouse_release(int code)
{
    uint32_t result = 0;
    INPUT in[1] = {};
    DWORD dwFlags;

    ZeroMemory(in, sizeof(in));

    if (code == LIBDRV_HID_BTN_LEFT)
        dwFlags = MOUSEEVENTF_LEFTUP;
    else if (code == LIBDRV_HID_BTN_RIGHT)
        dwFlags = MOUSEEVENTF_RIGHTUP;
    else if (code == LIBDRV_HID_BTN_MIDDLE)
        dwFlags = MOUSEEVENTF_MIDDLEUP;

    in[0].type = INPUT_MOUSE;
    in[0].mi.dwFlags = dwFlags;

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send mouse release error!\n");
}

static void hid_bridge_mouse_move(int dx, int dy)
{
    uint32_t result = 0;
    INPUT in[1] = {};

    ZeroMemory(in, sizeof(in));

    in[0].type = INPUT_MOUSE;
    in[0].mi.dx = dx;
    in[0].mi.dy = dy;
    in[0].mi.dwFlags = MOUSEEVENTF_MOVE;

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send mouse move error!\n");
}

static void hid_bridge_mouse_wheel(int d)
{
    uint32_t result = 0;
    INPUT in[1] = {};

    ZeroMemory(in, sizeof(in));

    in[0].type = INPUT_MOUSE;
    in[0].mi.mouseData = (DWORD)d * WHEEL_DELTA;
    in[0].mi.dwFlags = MOUSEEVENTF_WHEEL;

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send mouse scroll error!\n");
}

static void hid_bridge_mouse_hwheel(int d)
{
    uint32_t result = 0;
    INPUT in[1] = {};

    ZeroMemory(in, sizeof(in));

    in[0].type = INPUT_MOUSE;
    in[0].mi.mouseData = (DWORD)d * WHEEL_DELTA;
    in[0].mi.dwFlags = MOUSEEVENTF_HWHEEL;

    result = SendInput(1, in, sizeof(INPUT));
    if (result != 1)
        HID_BRIDGE_INFO("Send mouse h-scroll error!\n");
}

static void _hid_bridge_handle_kbd_event(struct kbd_key_event *event)
{
    if (!event)
        return;

    if (event->action != LIBDRV_HID_KBD_NO_EVENT)
    {
        int os_hid_code = libdrv_hid_to_hid_map[event->key];

        if (event->action == LIBDRV_HID_KBD_KEY_PRESSED)
        {
            if (event->modifier)
                hid_bridge_kbd_press(os_hid_code);
            else
                hid_bridge_kbd_press(os_hid_code);
        }
        else if (event->action == LIBDRV_HID_KBD_KEY_RELEASED)
            hid_bridge_kbd_release(os_hid_code);
    }

    // if(event->action != LIBDRV_HID_KBD_NO_EVENT)
    // {
    //     if(event->action == LIBDRV_HID_KBD_KEY_PRESSED)
    //         hid_bridge_kbd_press(event->key);
    // 	else if (event->action == LIBDRV_HID_KBD_KEY_RELEASED)
    // 		hid_bridge_kbd_release(event->key);
    // }
}

static void _hid_bridge_handle_mouse_event(struct mouse_event *event)
{
    if (!event)
        return;

    if (event->action != LIBDRV_HID_MOUSE_NO_EVENT)
    {
        if (event->action & LIBDRV_HID_MOUSE_BTN_PRESSED)
        {
            if (event->button & LIBDRV_HID_BTN_LEFT)
                hid_bridge_mouse_press(LIBDRV_HID_BTN_LEFT);
            if (event->button & LIBDRV_HID_BTN_RIGHT)
                hid_bridge_mouse_press(LIBDRV_HID_BTN_RIGHT);
            if (event->button & LIBDRV_HID_BTN_MIDDLE)
                hid_bridge_mouse_press(LIBDRV_HID_BTN_MIDDLE);
            if (event->button & LIBDRV_HID_BTN_WHEEL)
            {
                if (event->wheel == LIBDRV_HID_WHEEL_UP)
                    hid_bridge_mouse_wheel(1);
                else if (event->wheel == LIBDRV_HID_WHEEL_DOWN)
                    hid_bridge_mouse_wheel(-1);
                else if (event->wheel == LIBDRV_HID_WHEEL_LEFT)
                    hid_bridge_mouse_hwheel(-1);
                else if (event->wheel == LIBDRV_HID_WHEEL_RIGHT)
                    hid_bridge_mouse_hwheel(1);
            }
        }
        else if (event->action & LIBDRV_HID_MOUSE_BTN_RELEASED)
        {
            if (event->button & LIBDRV_HID_BTN_LEFT)
                hid_bridge_mouse_release(LIBDRV_HID_BTN_LEFT);
            if (event->button & LIBDRV_HID_BTN_RIGHT)
                hid_bridge_mouse_release(LIBDRV_HID_BTN_RIGHT);
            if (event->button & LIBDRV_HID_BTN_MIDDLE)
                hid_bridge_mouse_release(LIBDRV_HID_BTN_MIDDLE);
        }
        else if (event->action == LIBDRV_HID_MOUSE_MOVED)
        {
            // printf("Moved delta x:%d, delta y:%d\n", event->delta_x, event->delta_y);
            hid_bridge_mouse_move(event->delta_x, event->delta_y);
        }
    }
}

static size_t do_CMD_HID_NEW_EVENT(struct hid_bridge_msg *msg)
{
    // Origin: void hid_on_receiving_new_event(struct hid_event event)
    struct hid_event event = {0};
    uint64_t result = -1;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf_read_fixdata(varbuf, &event, sizeof(event)))
    {
        APP_ERR("CMD_HID_NEW_EVENT Request Failed: <event>\n");
        goto out;
    }
    varbuf_access_finish(varbuf);

    if (event.hid_dev == HID_KEYBOARD)
        _hid_bridge_handle_kbd_event(&event.kbd_event);
    else if (event.hid_dev == HID_MOUSE)
        _hid_bridge_handle_mouse_event(&event.mouse_event);

    result = 0;

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf_write_data(varbuf, &result, sizeof(result)))
        APP_ERR("CMD_HID_NEW_EVENT Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

static size_t do_CMD_HID_DTM_REQ_HIGH_PRIORITY(struct hid_bridge_msg *msg)
{
    // Origin: int hid_bridge_dtm_thread_req_priority(bool need_high_priority)
    uint32_t ret = 0;
    uint64_t result = -1;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    ret = hid_bridge_dtm_thread_req_priority(true);
    if (ret)
        result = 1;
    else
        result = 0;

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf_write_data(varbuf, &result, sizeof(result)))
        APP_ERR("CMD_HID_DTM_REQ_HIGH_PRIORITY Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#define HANDLE(hc)                  \
    case hc:                        \
        msg_data_sz = do_##hc(msg); \
        break

size_t handle_hid_bridge_function(struct hid_bridge_msg *msg, bool *out_valid_msg)
{
    size_t msg_data_sz = 0;
    bool valid_msg = true;

    switch (msg->cmd)
    {
        // Keyboard/mouse events
        HANDLE(CMD_HID_NEW_EVENT);

        HANDLE(CMD_HID_DTM_REQ_HIGH_PRIORITY);

    default:
        APP_ERR("Invalid Command!\n");
        valid_msg = false;
        break;
    }

    *out_valid_msg = valid_msg;

    return msg_data_sz;
}
