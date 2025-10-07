// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <gui/ui/libgui_headers.h>

#include "lib-gui-sdl-keycode.h"
#include "lib-gui-keycode.h"

static uint32_t libdrv_hid_to_sdl2_map[256] = 
{
    [LIBDRV_HID_KEY_ESC] SDLK_ESCAPE,
    [LIBDRV_HID_KEY_1] SDLK_1,
    [LIBDRV_HID_KEY_2] SDLK_2,
    [LIBDRV_HID_KEY_3] SDLK_3,
    [LIBDRV_HID_KEY_4] SDLK_4,
    [LIBDRV_HID_KEY_5] SDLK_5,
    [LIBDRV_HID_KEY_6] SDLK_6,
    [LIBDRV_HID_KEY_7] SDLK_7,
    [LIBDRV_HID_KEY_8] SDLK_8,
    [LIBDRV_HID_KEY_9] SDLK_9,
    [LIBDRV_HID_KEY_0] SDLK_0,
    [LIBDRV_HID_KEY_MINUS] SDLK_MINUS,
    [LIBDRV_HID_KEY_EQUAL] SDLK_EQUALS,
    [LIBDRV_HID_KEY_BACKSPACE] SDLK_BACKSPACE,
    [LIBDRV_HID_KEY_TAB] SDLK_TAB,
    [LIBDRV_HID_KEY_Q] SDLK_q,
    [LIBDRV_HID_KEY_W] SDLK_w,
    [LIBDRV_HID_KEY_E] SDLK_e,
    [LIBDRV_HID_KEY_R] SDLK_r,
    [LIBDRV_HID_KEY_T] SDLK_t,
    [LIBDRV_HID_KEY_Y] SDLK_y,
    [LIBDRV_HID_KEY_U] SDLK_u,
    [LIBDRV_HID_KEY_I] SDLK_i,
    [LIBDRV_HID_KEY_O] SDLK_o,
    [LIBDRV_HID_KEY_P] SDLK_p,
    [LIBDRV_HID_KEY_LEFT_SQUARE] SDLK_LEFTBRACKET,
    [LIBDRV_HID_KEY_RIGHT_SQUARE] SDLK_RIGHTBRACKET,
    [LIBDRV_HID_KEY_ENTER] SDLK_RETURN,
    [LIBDRV_HID_KEY_LEFT_CTRL] SDLK_LCTRL,
    [LIBDRV_HID_KEY_A] SDLK_a,
    [LIBDRV_HID_KEY_S] SDLK_s,
    [LIBDRV_HID_KEY_D] SDLK_d,
    [LIBDRV_HID_KEY_F] SDLK_f,
    [LIBDRV_HID_KEY_G] SDLK_g,
    [LIBDRV_HID_KEY_H] SDLK_h,
    [LIBDRV_HID_KEY_J] SDLK_j,
    [LIBDRV_HID_KEY_K] SDLK_k,
    [LIBDRV_HID_KEY_L] SDLK_l,
    [LIBDRV_HID_KEY_SEMICOLON] SDLK_SEMICOLON,
    [LIBDRV_HID_KEY_QUOTE] SDLK_QUOTE,
    [LIBDRV_HID_KEY_BACKTICK] SDLK_BACKQUOTE,
    [LIBDRV_HID_KEY_LEFT_SHIFT] SDLK_LSHIFT,
    [LIBDRV_HID_KEY_BACKSLASH] SDLK_BACKSLASH,
    [LIBDRV_HID_KEY_Z] SDLK_z,
    [LIBDRV_HID_KEY_X] SDLK_x,
    [LIBDRV_HID_KEY_C] SDLK_c,
    [LIBDRV_HID_KEY_V] SDLK_v,
    [LIBDRV_HID_KEY_B] SDLK_b,
    [LIBDRV_HID_KEY_N] SDLK_n,
    [LIBDRV_HID_KEY_M] SDLK_m,
    [LIBDRV_HID_KEY_COMMA] SDLK_COMMA,
    [LIBDRV_HID_KEY_PERIOD] SDLK_PERIOD,
    [LIBDRV_HID_KEY_SLASH] SDLK_SLASH,
    [LIBDRV_HID_KEY_RIGHT_SHIFT] SDLK_RSHIFT,
    [LIBDRV_HID_KEYPAD_ASTERIX] SDLK_KP_MULTIPLY,
    [LIBDRV_HID_KEY_LEFT_ALT] SDLK_LALT,
    [LIBDRV_HID_KEY_SPACE] SDLK_SPACE,
    [LIBDRV_HID_KEY_CAPS_LOCK] SDLK_CAPSLOCK,
    [LIBDRV_HID_KEY_F1] SDLK_F1,
    [LIBDRV_HID_KEY_F2] SDLK_F2,
    [LIBDRV_HID_KEY_F3] SDLK_F3,
    [LIBDRV_HID_KEY_F4] SDLK_F4,
    [LIBDRV_HID_KEY_F5] SDLK_F5,
    [LIBDRV_HID_KEY_F6] SDLK_F6,
    [LIBDRV_HID_KEY_F7] SDLK_F7,
    [LIBDRV_HID_KEY_F8] SDLK_F8,
    [LIBDRV_HID_KEY_F9] SDLK_F9,
    [LIBDRV_HID_KEY_F10] SDLK_F10,
    [LIBDRV_HID_KEY_NUM_LOCK] SDLK_NUMLOCKCLEAR,
    [LIBDRV_HID_KEY_SCROLL_LOCK] SDLK_SCROLLLOCK,
    [LIBDRV_HID_KEYPAD_7] SDLK_KP_7,
    [LIBDRV_HID_KEYPAD_8] SDLK_KP_8,
    [LIBDRV_HID_KEYPAD_9] SDLK_KP_9,
    [LIBDRV_HID_KEYPAD_MINUS] SDLK_KP_MINUS,
    [LIBDRV_HID_KEYPAD_4] SDLK_KP_4,
    [LIBDRV_HID_KEYPAD_5] SDLK_KP_5,
    [LIBDRV_HID_KEYPAD_6] SDLK_KP_6,
    [LIBDRV_HID_KEYPAD_PLUS] SDLK_KP_PLUS,
    [LIBDRV_HID_KEYPAD_1] SDLK_KP_1,
    [LIBDRV_HID_KEYPAD_2] SDLK_KP_2,
    [LIBDRV_HID_KEYPAD_3] SDLK_KP_3,
    [LIBDRV_HID_KEYPAD_0] SDLK_KP_0,
    [LIBDRV_HID_KEYPAD_PERIOD] SDLK_KP_PERIOD,
    [LIBDRV_HID_KEY_F11] SDLK_F11,
    [LIBDRV_HID_KEY_F12] SDLK_F12,
    [LIBDRV_HID_KEY_PRINTSCREEN] SDLK_PRINTSCREEN,
    [LIBDRV_HID_KEYPAD_ENTER] SDLK_KP_ENTER,
    [LIBDRV_HID_KEY_RIGHT_CTRL] SDLK_RCTRL,
    [LIBDRV_HID_KEYPAD_SLASH] SDLK_KP_DIVIDE,
    [LIBDRV_HID_KEY_RIGHT_ALT] SDLK_RALT,
    [LIBDRV_HID_KEY_HOME] SDLK_HOME,
    [LIBDRV_HID_KEY_UP] SDLK_UP,
    [LIBDRV_HID_KEY_PAGE_UP] SDLK_PAGEUP,
    [LIBDRV_HID_KEY_LEFT] SDLK_LEFT,
    [LIBDRV_HID_KEY_RIGHT] SDLK_RIGHT,
    [LIBDRV_HID_KEY_END] SDLK_END,
    [LIBDRV_HID_KEY_DOWN] SDLK_DOWN,
    [LIBDRV_HID_KEY_PAGE_DOWN] SDLK_PAGEDOWN,
    [LIBDRV_HID_KEY_INSERT] SDLK_INSERT,
    [LIBDRV_HID_KEY_DELETE] SDLK_DELETE,
    [LIBDRV_HID_KEY_LEFT_GUI] SDLK_LGUI,
    [LIBDRV_HID_KEY_RIGHT_GUI] SDLK_RGUI
};

/**
 * Convert the key code LV_KEY_... "codes" or leave them if they are not control characters
 * @param sdl_key the key code
 * @return
 */
static uint32_t sdl2_keycode_to_lvgl_keycode(uint32_t sdl_key)
{
    /*Remap some key to LV_KEY_... to manage groups*/
    switch(sdl_key) {
        case SDLK_RIGHT:
        case SDLK_KP_PLUS:
            return LV_KEY_RIGHT;

        case SDLK_LEFT:
        case SDLK_KP_MINUS:
            return LV_KEY_LEFT;

        case SDLK_UP:
            return LV_KEY_UP;

        case SDLK_DOWN:
            return LV_KEY_DOWN;

        case SDLK_ESCAPE:
            return LV_KEY_ESC;

#ifdef  LV_KEY_BACKSPACE        /*For backward compatibility*/
        case SDLK_BACKSPACE:
            return LV_KEY_BACKSPACE;
#endif

#ifdef  LV_KEY_DEL        /*For backward compatibility*/
        case SDLK_DELETE:
            return LV_KEY_DEL;
#endif
        case SDLK_KP_ENTER:
        case '\r':
            return LV_KEY_ENTER;

        case SDLK_PAGEDOWN:
            return LV_KEY_NEXT;

        case SDLK_PAGEUP:
            return LV_KEY_PREV;

        case SDLK_HOME:
            return LV_KEY_HOME;
        
        case SDLK_END:
            return LV_KEY_END;

        case SDLK_INSERT:
            return 0; // LVGL does not support INSERT key

        default:
            return sdl_key;
    }
}


uint32_t libhid_keycode_to_lvgl_keycode(uint32_t libhid_keycode)
{
    return sdl2_keycode_to_lvgl_keycode(libdrv_hid_to_sdl2_map[libhid_keycode]);
}