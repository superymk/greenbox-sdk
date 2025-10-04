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

#include <sap_headers.h>
#include <palddk.h>

#include "rdesktop_hid.h"

static int realWidth, realHeight, bytesPerPixel, rowStride;
static char *sdlPixels = NULL;

// Keyboard
static uint8_t kbd_event_to_character[256] =
    {
        [LIBDRV_HID_KEY_A] 'a',
        [LIBDRV_HID_KEY_B] 'b',
        [LIBDRV_HID_KEY_C] 'c',
        [LIBDRV_HID_KEY_D] 'd',
        [LIBDRV_HID_KEY_E] 'e',
        [LIBDRV_HID_KEY_F] 'f',
        [LIBDRV_HID_KEY_G] 'g',
        [LIBDRV_HID_KEY_H] 'h',
        [LIBDRV_HID_KEY_I] 'i',
        [LIBDRV_HID_KEY_J] 'j',
        [LIBDRV_HID_KEY_K] 'k',
        [LIBDRV_HID_KEY_L] 'l',
        [LIBDRV_HID_KEY_M] 'm',
        [LIBDRV_HID_KEY_N] 'n',
        [LIBDRV_HID_KEY_O] 'o',
        [LIBDRV_HID_KEY_P] 'p',
        [LIBDRV_HID_KEY_Q] 'q',
        [LIBDRV_HID_KEY_R] 'r',
        [LIBDRV_HID_KEY_S] 's',
        [LIBDRV_HID_KEY_T] 't',
        [LIBDRV_HID_KEY_U] 'u',
        [LIBDRV_HID_KEY_V] 'v',
        [LIBDRV_HID_KEY_W] 'w',
        [LIBDRV_HID_KEY_X] 'x',
        [LIBDRV_HID_KEY_Y] 'y',
        [LIBDRV_HID_KEY_Z] 'z',
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
        [LIBDRV_HID_KEY_MINUS] '-',
        [LIBDRV_HID_KEY_EQUAL] '=',
        [LIBDRV_HID_KEY_LEFT_BRACE] '(',
        [LIBDRV_HID_KEY_RIGHT_BRACE] ')',
        [LIBDRV_HID_KEY_BACKSLASH] '\\',
        [LIBDRV_HID_KEY_SEMICOLON] ';',
        [LIBDRV_HID_KEY_QUOTE] '\'',
        [LIBDRV_HID_KEY_TILDE] '~',
        [LIBDRV_HID_KEY_COMMA] ',',
        [LIBDRV_HID_KEY_PERIOD] '.',
        [LIBDRV_HID_KEY_SLASH] '/',
        [LIBDRV_HID_KEY_LEFT_SQUARE] '[',
        [LIBDRV_HID_KEY_RIGHT_SQUARE] ']'};

// Mouse
static int cursor_x = 0;
static int cursor_y = 0;
static bool clicked_btn_left = false;
static bool clicked_btn_middle = false;
static bool clicked_btn_right = false;
static bool clicked_btn_wheel = false;

static rfbKeySym send_kbdevent(struct kbd_key_event *e)
{
    rfbKeySym k = 0;
    uint8_t key = e->key;

    switch (key)
    {
    case LIBDRV_HID_KEY_BACKSPACE:
        k = XK_BackSpace;
        break;
    case LIBDRV_HID_KEY_TAB:
        k = XK_Tab;
        break;
    // case KEY_CLEAR: k = XK_Clear; break;
    case LIBDRV_HID_KEY_ENTER:
        k = XK_Return;
        break;
    case LIBDRV_HID_KEY_PAUSE:
        k = XK_Pause;
        break;
    case LIBDRV_HID_KEY_ESC:
        k = XK_Escape;
        break;
    case LIBDRV_HID_KEY_SPACE:
        k = XK_space;
        break;
    case LIBDRV_HID_KEY_DELETE:
        k = XK_Delete;
        break;
    case LIBDRV_HID_KEYPAD_0:
        k = XK_KP_0;
        break;
    case LIBDRV_HID_KEYPAD_1:
        k = XK_KP_1;
        break;
    case LIBDRV_HID_KEYPAD_2:
        k = XK_KP_2;
        break;
    case LIBDRV_HID_KEYPAD_3:
        k = XK_KP_3;
        break;
    case LIBDRV_HID_KEYPAD_4:
        k = XK_KP_4;
        break;
    case LIBDRV_HID_KEYPAD_5:
        k = XK_KP_5;
        break;
    case LIBDRV_HID_KEYPAD_6:
        k = XK_KP_6;
        break;
    case LIBDRV_HID_KEYPAD_7:
        k = XK_KP_7;
        break;
    case LIBDRV_HID_KEYPAD_8:
        k = XK_KP_8;
        break;
    case LIBDRV_HID_KEYPAD_9:
        k = XK_KP_9;
        break;
    case LIBDRV_HID_KEYPAD_PERIOD:
        k = XK_KP_Decimal;
        break;
    case LIBDRV_HID_KEYPAD_SLASH:
        k = XK_KP_Divide;
        break;
    case LIBDRV_HID_KEYPAD_ASTERIX:
        k = XK_KP_Multiply;
        break;
    case LIBDRV_HID_KEYPAD_MINUS:
        k = XK_KP_Subtract;
        break;
    case LIBDRV_HID_KEYPAD_PLUS:
        k = XK_KP_Add;
        break;
    case LIBDRV_HID_KEYPAD_ENTER:
        k = XK_KP_Enter;
        break;
    // case KEY_KP_EQUALS: k = XK_KP_Equal; break;
    case LIBDRV_HID_KEY_UP:
        k = XK_Up;
        break;
    case LIBDRV_HID_KEY_DOWN:
        k = XK_Down;
        break;
    case LIBDRV_HID_KEY_RIGHT:
        k = XK_Right;
        break;
    case LIBDRV_HID_KEY_LEFT:
        k = XK_Left;
        break;
    case LIBDRV_HID_KEY_INSERT:
        k = XK_Insert;
        break;
    case LIBDRV_HID_KEY_HOME:
        k = XK_Home;
        break;
    case LIBDRV_HID_KEY_END:
        k = XK_End;
        break;
    case LIBDRV_HID_KEY_PAGE_UP:
        k = XK_Page_Up;
        break;
    case LIBDRV_HID_KEY_PAGE_DOWN:
        k = XK_Page_Down;
        break;
    case LIBDRV_HID_KEY_F1:
        k = XK_F1;
        break;
    case LIBDRV_HID_KEY_F2:
        k = XK_F2;
        break;
    case LIBDRV_HID_KEY_F3:
        k = XK_F3;
        break;
    case LIBDRV_HID_KEY_F4:
        k = XK_F4;
        break;
    case LIBDRV_HID_KEY_F5:
        k = XK_F5;
        break;
    case LIBDRV_HID_KEY_F6:
        k = XK_F6;
        break;
    case LIBDRV_HID_KEY_F7:
        k = XK_F7;
        break;
    case LIBDRV_HID_KEY_F8:
        k = XK_F8;
        break;
    case LIBDRV_HID_KEY_F9:
        k = XK_F9;
        break;
    case LIBDRV_HID_KEY_F10:
        k = XK_F10;
        break;
    case LIBDRV_HID_KEY_F11:
        k = XK_F11;
        break;
    case LIBDRV_HID_KEY_F12:
        k = XK_F12;
        break;
        // case KEY_F13:
        // 	k = XK_F13;
        // 	break;
        // case KEY_F14:
        // 	k = XK_F14;
        // 	break;
        // case KEY_F15:
        // 	k = XK_F15;
        // 	break;

    case LIBDRV_HID_KEY_LEFT_SHIFT:
        k = XK_Shift_L;
        break;
    case LIBDRV_HID_KEY_RIGHT_SHIFT:
        k = XK_Shift_R;
        break;
    case LIBDRV_HID_KEY_LEFT_CTRL:
        k = XK_Control_L;
        break;
    case LIBDRV_HID_KEY_RIGHT_CTRL:
        k = XK_Control_R;
        break;
    case LIBDRV_HID_KEY_LEFT_ALT:
        k = XK_Alt_L;
        break;
    case LIBDRV_HID_KEY_RIGHT_ALT:
        k = XK_Alt_R;
        break;
    case LIBDRV_HID_KEY_LEFT_GUI:
        k = XK_Meta_L;
        break;
    case LIBDRV_HID_KEY_RIGHT_GUI:
        k = XK_Meta_R;
        break;

    case LIBDRV_HID_KEY_NUM_LOCK:
        k = XK_Num_Lock;
        break;
    case LIBDRV_HID_KEY_CAPS_LOCK:
        k = XK_Caps_Lock;
        break;
    case LIBDRV_HID_KEY_SCROLL_LOCK:
        k = XK_Scroll_Lock;
        break;
#if 0
		case KEY_COMPOSE: k = XK_Compose; break;
#endif
    // case KEY_MODE: k = XK_Mode_switch; break;
    // case KEY_HELP: k = XK_Help; break;
    case LIBDRV_HID_KEY_PRINTSCREEN:
        k = XK_Print;
        break;
    // case KEY_SYSREQ: k = XK_Sys_Req; break;
    //  case LIBDRV_HID_KEY_PAUSE: k = XK_Break; break;
    default:
        break;
    }

    /* both SDL and X11 keysyms match ASCII in the range 0x01-0x7f */
    if (k == 0)
    {
        k = key = kbd_event_to_character[e->key];
        if (key > 0x0 && key < 0x100)
        {
            if (e->modifier & (LIBDRV_HID_MOD_LEFT_SHIFT | LIBDRV_HID_MOD_RIGHT_SHIFT))
            {
                if (k >= '1' && k <= '9')
                    k &= ~0x10;
                else if (k >= 'a' && k <= 'z')
                    k &= ~0x20;
            }
        }
    }
    if (k == 0)
    {
        if (key < 0x100)
            k = key;
        else
            rfbClientLog("Unknown keysym: %d\n", key);
    }

    return k;
}

void rfb_send_mouse_pos(rfbClient *cl, int pos_x, int pos_y)
{
    rfbBool status;
    int buttonMask = 0;

    if (sdlPixels)
    {
        pos_x = pos_x * cl->width / realWidth;
        pos_y = pos_y * cl->height / realHeight;
    }

    status = SendPointerEvent(cl, pos_x, pos_y, buttonMask);
    (void)status;
}

void rdesktop_handle_kbd_event(rfbClient *cl, struct kbd_key_event *event)
{
    rfbKeySym k = 0;

    if (!event)
        return;

    // Send key event
    k = send_kbdevent(event);

    if (event->action == LIBDRV_HID_KBD_KEY_PRESSED)
        SendKeyEvent(cl, k, TRUE);
    else if (event->action == LIBDRV_HID_KBD_KEY_RELEASED)
        SendKeyEvent(cl, k, FALSE);
}

void rdesktop_handle_mouse_event(rfbClient *cl, struct mouse_event *event)
{
    rfbBool status;
    int x, y;
    int buttonMask = 0;

    if (event->action == LIBDRV_HID_MOUSE_NO_EVENT)
        return;

    // Handle event change
    if (event->action == LIBDRV_HID_MOUSE_MOVED)
    {
        // cursor_x += event->delta_x;
        // cursor_y += event->delta_y;

        // // Limit the cursor position to be within the window
        // if (cursor_x < 0)
        //     cursor_x = 0;
        // if (cursor_x > realWidth)
        //     cursor_x = realWidth;
        // if (cursor_y < 0)
        //     cursor_y = 0;
        // if (cursor_y > realHeight)
        //     cursor_y = realHeight;

        // Send current mouse position to the RFB server.
        libgui_mouse_get_pos((uint16_t*)&cursor_x, (uint16_t*)&cursor_y);
    }
    else
    {
        int btn = event->button;
        bool clicked;

        if (event->action & LIBDRV_HID_MOUSE_BTN_PRESSED)
        {
            clicked = true;
        }
        else if (event->action & LIBDRV_HID_MOUSE_BTN_RELEASED)
        {
            clicked = false;
        }

        if (btn & LIBDRV_HID_BTN_LEFT)
            clicked_btn_left = clicked;
        if (btn & LIBDRV_HID_BTN_RIGHT)
            clicked_btn_right = clicked;
        if (btn & LIBDRV_HID_BTN_MIDDLE)
            clicked_btn_middle = clicked;
        if (btn & LIBDRV_HID_BTN_WHEEL)
            clicked_btn_wheel = clicked;
    }

    // Send mouse status to server
    {
        int btn = event->button;

        // Handle mouse pointer status
        x = cursor_x;
        y = cursor_y;

        if (sdlPixels)
        {
            x = x * cl->width / realWidth;
            y = y * cl->height / realHeight;
        }

        // Handle mouse wheel events
        if (btn & LIBDRV_HID_BTN_WHEEL)
        {
            if (event->wheel == LIBDRV_HID_WHEEL_UP)
            {
                if (clicked_btn_wheel)
                    buttonMask |= rfbWheelUpMask;
                else
                    buttonMask &= ~rfbWheelUpMask;
            }
            else if (event->wheel == LIBDRV_HID_WHEEL_DOWN)
            {
                if (clicked_btn_wheel)
                    buttonMask |= rfbWheelDownMask;
                else
                    buttonMask &= ~rfbWheelDownMask;
            }
        }

        // Handle button press status
        if (clicked_btn_left)
            buttonMask |= rfbButton1Mask;
        else
            buttonMask &= ~rfbButton1Mask;

        if (clicked_btn_middle)
            buttonMask |= rfbButton2Mask;
        else
            buttonMask &= ~rfbButton2Mask;

        if (clicked_btn_right)
            buttonMask |= rfbButton3Mask;
        else
            buttonMask &= ~rfbButton3Mask;

        // // Handle button press events
        // for (i = 0; buttonMapping[i].sdl; i++)
        // {
        // 	if (btn == buttonMapping[i].sdl)
        // 	{
        // 		btn = buttonMapping[i].rfb;
        // 		if (event->action & LIBDRV_HID_MOUSE_BTN_PRESSED)
        // 			buttonMask |= btn;
        // 		else if (event->action & LIBDRV_HID_MOUSE_BTN_RELEASED)
        // 			buttonMask &= ~btn;
        // 		break;
        // 	}
        // }
    }

    status = SendPointerEvent(cl, x, y, buttonMask);
    (void)status;
    buttonMask &= ~(rfbButton4Mask | rfbButton5Mask);

    // If the mouse event is wheel event, then we need to notify VNC server to clear the mask
    if (event->button & LIBDRV_HID_BTN_WHEEL)
    {
        status = SendPointerEvent(cl, x, y, 0);
        (void)status;
    }
}

void rdesktop_hid_init(int in_realWidth, int in_realHeight, int in_bytesPerPixel, int in_rowStride, char *in_sdlPixels)
{
    realWidth = in_realWidth;
    realHeight = in_realHeight;
    bytesPerPixel = in_bytesPerPixel;
    rowStride = in_rowStride;
    sdlPixels = in_sdlPixels;
}