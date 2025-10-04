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

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <gui/ui/libgui_headers.h>

#include "lib-gui-keycode.h"
#include "lib-gui-sdl-keycode.h"


extern SAP_STATUS libgui_display_init(struct libgui_config *cfg);
extern void libgui_display_fini(void);


static lv_indev_t *kbd_indev = NULL;
static lv_indev_t *mouse_indev = NULL;
static lv_obj_t *cursor_obj = NULL;

static uint32_t vinfo_xres = 0;
static uint32_t vinfo_yres = 0;

//// Mouse configs
static uint32_t lines_per_wheel = 0;

// For the generic mouse driver
static bool mouse_state_valid = true;
static bool left_button_down = false;
static int32_t last_x = 0;
static int32_t last_y = 0;
static int16_t wheel_diff = 0;
static lv_indev_state_t wheel_state = LV_INDEV_STATE_RELEASED;

// For the generic kbd driver
static bool kbd_state_valid = true;
static bool key_pressed = false;
struct kbd_key_event last_event;

static void _libgui_mouse_read(lv_indev_t * indev, lv_indev_data_t *data)
{
    if (!mouse_state_valid)
        return;

    mouse_state_valid = false;
    mfence();

    /*Store the collected data*/
    data->point.x = (lv_coord_t)last_x;
    data->point.y = (lv_coord_t)last_y;
    data->state = left_button_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    // // [TODO] Not support right button and wheel well
    // data->enc_diff = wheel_diff;
    // wheel_diff = 0;

    // if (touchpad_pressed)
    // {
    //     data->point.x = touchpad_x;
    //     data->point.y = touchpad_y;
    //     data->state = LV_INDEV_STATE_PRESSED;
    // }
    // else
    // {
    //     data->state = LV_INDEV_STATE_RELEASED;
    // }
}

static void _libgui_mousewheel_read(lv_indev_t * indev, lv_indev_data_t *data)
{
    (void)indev; /*Unused*/

    data->state = wheel_state;
    data->enc_diff = wheel_diff;
    wheel_diff = 0;
}

static uint32_t _shift_map[256] =
    {
        // Lowercase letters
        ['a'] 'A',
        ['b'] 'B',
        ['c'] 'C',
        ['d'] 'D',
        ['e'] 'E',
        ['f'] 'F',
        ['g'] 'G',
        ['h'] 'H',
        ['i'] 'I',
        ['j'] 'J',
        ['k'] 'K',
        ['l'] 'L',
        ['m'] 'M',
        ['n'] 'N',
        ['o'] 'O',
        ['p'] 'P',
        ['q'] 'Q',
        ['r'] 'R',
        ['s'] 'S',
        ['t'] 'T',
        ['u'] 'U',
        ['v'] 'V',
        ['w'] 'W',
        ['x'] 'X',
        ['y'] 'Y',
        ['z'] 'Z',

        // Uppercase letters
        ['A'] 'a',
        ['B'] 'b',
        ['C'] 'c',
        ['D'] 'd',
        ['E'] 'e',
        ['F'] 'f',
        ['G'] 'g',
        ['H'] 'h',
        ['I'] 'i',
        ['J'] 'j',
        ['K'] 'k',
        ['L'] 'l',
        ['M'] 'm',
        ['N'] 'n',
        ['O'] 'o',
        ['P'] 'p',
        ['Q'] 'q',
        ['R'] 'r',
        ['S'] 's',
        ['T'] 't',
        ['U'] 'u',
        ['V'] 'v',
        ['W'] 'w',
        ['X'] 'x',
        ['Y'] 'y',
        ['Z'] 'z',

        // Numbers
        ['0'] ')',
        ['1'] '!',
        ['2'] '@',
        ['3'] '#',
        ['4'] '$',
        ['5'] '%',
        ['6'] '^',
        ['7'] '&',
        ['8'] '*',
        ['9'] '(',

        // Symbols
        ['`'] '~',
        ['-'] '_',
        ['='] '+',
        ['['] '{',
        [']'] '}',
        ['\\'] '|',
        [';'] ':',
        ['\''] '\"',
        [','] '<',
        ['.'] '>',
        ['/'] '?',

        // Add more keys here as needed
};

static uint32_t _lvgl_numlock_on_map[256] =
    {
        [LIBDRV_HID_KEYPAD_SLASH] '/',
        [LIBDRV_HID_KEYPAD_ASTERIX] '*',
        [LIBDRV_HID_KEYPAD_MINUS] '-',
        [LIBDRV_HID_KEYPAD_PLUS] '+',
        [LIBDRV_HID_KEYPAD_ENTER] LV_KEY_ENTER,
        [LIBDRV_HID_KEYPAD_1] '1',
        [LIBDRV_HID_KEYPAD_2] '2',
        [LIBDRV_HID_KEYPAD_3] '3',
        [LIBDRV_HID_KEYPAD_4] '4',
        [LIBDRV_HID_KEYPAD_5] '5',
        [LIBDRV_HID_KEYPAD_6] '6',
        [LIBDRV_HID_KEYPAD_7] '7',
        [LIBDRV_HID_KEYPAD_8] '8',
        [LIBDRV_HID_KEYPAD_9] '9',
        [LIBDRV_HID_KEYPAD_0] '0',
        [LIBDRV_HID_KEYPAD_PERIOD] '.',
};

static uint32_t _lvgl_numlock_off_map[256] =
    {
        [LIBDRV_HID_KEYPAD_SLASH] '/',
        [LIBDRV_HID_KEYPAD_ASTERIX] '*',
        [LIBDRV_HID_KEYPAD_MINUS] '-',
        [LIBDRV_HID_KEYPAD_PLUS] '+',
        [LIBDRV_HID_KEYPAD_ENTER] LV_KEY_ENTER,
        [LIBDRV_HID_KEYPAD_1] LV_KEY_END,
        [LIBDRV_HID_KEYPAD_2] LV_KEY_DOWN,
        [LIBDRV_HID_KEYPAD_3] LV_KEY_NEXT, // SDLK_PAGEDOWN
        [LIBDRV_HID_KEYPAD_4] LV_KEY_LEFT,
        [LIBDRV_HID_KEYPAD_5] 0,
        [LIBDRV_HID_KEYPAD_6] LV_KEY_RIGHT,
        [LIBDRV_HID_KEYPAD_7] LV_KEY_HOME,
        [LIBDRV_HID_KEYPAD_8] LV_KEY_UP,
        [LIBDRV_HID_KEYPAD_9] LV_KEY_PREV, // SDLK_PAGEUP
        [LIBDRV_HID_KEYPAD_0] SDLK_INSERT, // INSERT in lvgl has the same keycode with SDLK_INSERT
        [LIBDRV_HID_KEYPAD_PERIOD] LV_KEY_DEL,
};

static void _libgui_keyboard_read(lv_indev_t * indev, lv_indev_data_t *data)
{
    if (!kbd_state_valid)
        return;

    kbd_state_valid = false;
    mfence();

    // Keep the modifiers, but drop the modifier keys
    if ((last_event.key == LIBDRV_HID_KEY_CAPS_LOCK) || (last_event.key == LIBDRV_HID_KEY_NUM_LOCK) || (last_event.key == LIBDRV_HID_KEY_SCROLL_LOCK))
        return;
    if ((last_event.key == LIBDRV_HID_KEY_LEFT_CTRL) || (last_event.key == LIBDRV_HID_KEY_LEFT_SHIFT) || (last_event.key == LIBDRV_HID_KEY_LEFT_ALT) || (last_event.key == LIBDRV_HID_KEY_LEFT_GUI))
        return;
    if ((last_event.key == LIBDRV_HID_KEY_RIGHT_CTRL) || (last_event.key == LIBDRV_HID_KEY_RIGHT_SHIFT) || (last_event.key == LIBDRV_HID_KEY_RIGHT_ALT) || (last_event.key == LIBDRV_HID_KEY_RIGHT_GUI))
        return;

    // Convert Libhid keycode to LVGL keycode
    data->key = libhid_keycode_to_lvgl_keycode(last_event.key);

    // Handle CAPSLOCK
    if (last_event.modifier & LIBDRV_HID_MOD_CAPS_LOCK)
    {
        // Handle CAPSLOCK
        if (data->key >= 'a' && data->key <= 'z')
        {
            data->key = data->key - ('a' - 'A');
        }
    }

    // Handle NUMSLOCK
    if (last_event.modifier & LIBDRV_HID_MOD_NUM_LOCK)
    {
        // Handle NUMSLOCK on
        uint32_t new_key = _lvgl_numlock_on_map[(uint8_t)last_event.key];
        if (new_key != 0)
            data->key = new_key;
    }
    else
    {
        // Handle NUMSLOCK off
        uint32_t new_key = _lvgl_numlock_off_map[(uint8_t)last_event.key];
        if (new_key != 0)
            data->key = new_key;
    }

    // Handle Shift
    if ((last_event.modifier & LIBDRV_HID_MOD_LEFT_SHIFT) || (last_event.modifier & LIBDRV_HID_MOD_RIGHT_SHIFT))
    {
        uint32_t new_key = _shift_map[(uint8_t)data->key];
        if (new_key != 0)
            data->key = new_key;
    }

    if (last_event.action == LIBDRV_HID_KBD_KEY_PRESSED)
        data->state = LV_INDEV_STATE_PRESSED;
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
static uint32_t _custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}


/********* Public Functions *********/
//! \brief LibGUI's call back function to handle keyboard input events.
//! This function should be registered in the keyboard driver to listen on keyboard events
void libgui_kbd_listener(struct kbd_key_event *event)
{
    // Sanity checks
    if (!event)
        return;

    if (event->action == LIBDRV_HID_KBD_NO_EVENT)
        return;

    memcpy(&last_event, event, sizeof(struct kbd_key_event));

    // last_key = event->key;

    // if(event->action == LIBDRV_HID_KBD_KEY_PRESSED)
    //     key_pressed = true;
    // else
    //     key_pressed = false;

    // Flag the new keyboard state
    kbd_state_valid = true;
}

//! \brief LibGUI's call back function to handle mouse input events.
//! This function should be registered in the mouse driver to listen on mouse events
void libgui_mouse_listener(struct mouse_event *event)
{
    // Sanity checks
    if (!event)
        return;

    if (event->action == LIBDRV_HID_MOUSE_NO_EVENT)
        return;

    if (event->action & LIBDRV_HID_MOUSE_BTN_PRESSED)
    {
        if (event->button & LIBDRV_HID_BTN_LEFT)
            left_button_down = true;
        if (event->button & LIBDRV_HID_BTN_WHEEL)
            wheel_state = LV_INDEV_STATE_PRESSED;
    }

    if (event->action & LIBDRV_HID_MOUSE_BTN_RELEASED)
    {
        if (event->button & LIBDRV_HID_BTN_LEFT)
            left_button_down = false;
        if (event->button & LIBDRV_HID_BTN_WHEEL)
            wheel_state = LV_INDEV_STATE_RELEASED;
    }

    if (event->action == LIBDRV_HID_MOUSE_MOVED)
    {
        last_x += event->delta_x;
        last_y += event->delta_y;

        // Limit the mouse pointer is inside the GUI region
        if (last_x >= (int32_t)vinfo_xres)
            last_x = (int32_t)vinfo_xres;
        if (last_x <= 0)
            last_x = 0;

        if (last_y >= (int32_t)vinfo_yres)
            last_y = (int32_t)vinfo_yres;
        if (last_y <= 0)
            last_y = 0;
    }

    // Wheel up/down/left/right
    if (event->button & LIBDRV_HID_BTN_WHEEL)
    {
        if (event->wheel == LIBDRV_HID_WHEEL_UP)
        {
            // wheel_diff += lines_per_wheel;
            wheel_diff--;
        }
        else if (event->wheel == LIBDRV_HID_WHEEL_DOWN)
        {
            // wheel_diff += -lines_per_wheel;
            wheel_diff++;
        }
        else if (event->wheel == LIBDRV_HID_WHEEL_LEFT)
            GUI_INFO("Mouse Wheel Left - Not Implemented\n");
        else if (event->wheel == LIBDRV_HID_WHEEL_RIGHT)
            GUI_INFO("Mouse Wheel Right - Not Implemented\n");
    }

    // Flag the new mouse state
    mouse_state_valid = true;
}

SAP_STATUS libgui_init(struct libgui_config *cfg, bool create_mouse_cursor)
{
    SAP_STATUS status = SUCCESS;

    // Check: <cfg> must be valid
    if (!cfg)
        return EINVAL;

    if (cfg->fb_vaddr == INVALID_VADDR)
        return EINVAL;

    // Initialize window canvas
    // LittlevGL init
    lv_init();

    // Tell LVGL how to read time
    lv_tick_set_cb(_custom_tick_get);

    vinfo_xres = cfg->fb_width;
    vinfo_yres = cfg->fb_height;

    // Init the display
    status = libgui_display_init(cfg);
    if (FAIL(status))
    {
        GUI_ERR("Init Lib-GUI error: Failed to init display!\n");
        status = EINTERNAL;
        goto init_disp_err;
    }

    // Create the group
    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);

    // Register keyboard
    kbd_indev = lv_indev_create();
    if (!kbd_indev)
    {
        GUI_ERR("Init Lib-GUI error: Failed to init keyboard!\n");
        status = EINTERNAL;
        goto init_kbd_err;
    }

    lv_indev_set_type(kbd_indev, LV_INDEV_TYPE_KEYPAD);
    // lv_indev_set_user_data(kbd_indev, indev_ctx);
    // lv_indev_add_event_cb(kbd_indev, _simple_text_input_event_cb, LV_EVENT_DELETE, kbd_indev);
    lv_indev_set_read_cb(kbd_indev, _libgui_keyboard_read);

    lv_indev_set_group(kbd_indev, g);

    // Register mouse
    mouse_indev = lv_indev_create();
    if (!mouse_indev)
    {
        GUI_ERR("Init Lib-GUI error: Failed to init mouse!\n");
        status = EINTERNAL;
        goto init_mouse_err;
    }

    lv_indev_set_type(mouse_indev, LV_INDEV_TYPE_POINTER);
    // lv_indev_set_user_data(mouse_indev, indev_ctx);
    // lv_indev_add_event_cb(mouse_indev, _simple_text_input_event_cb, LV_EVENT_DELETE, mouse_indev);
    lv_indev_set_read_cb(mouse_indev, _libgui_mouse_read);

    //// Configure mouse
    lines_per_wheel = cfg->lines_per_wheel;

    // Set a cursor for the mouse
    if (create_mouse_cursor)
    {
        LV_IMG_DECLARE(mouse_cursor_icon)
        cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
        if (!cursor_obj)
        {
            GUI_ERR("Init Lib-GUI error: Failed to init cursor!\n");
            status = EINTERNAL;
            goto init_cursor_err;
        }
        lv_img_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
        lv_indev_set_cursor(mouse_indev, cursor_obj);   /*Connect the image  object to the driver*/
    }

    // Register mouse wheel
    lv_indev_t *enc_indev = lv_indev_create();
    if (!enc_indev)
    {
        GUI_ERR("Init Lib-GUI error: Failed to init mouse wheel!\n");
        status = EINTERNAL;
        goto init_mousewheel_err;
    }

    lv_indev_set_type(enc_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(enc_indev, _libgui_mousewheel_read);

    lv_indev_set_group(enc_indev, g);

    GUI_INFO("Initialize Lib-GUI Success\n");
    return SUCCESS;

init_mousewheel_err:
init_cursor_err:
init_mouse_err:
init_kbd_err:
init_disp_err:
    return status;
}

void libgui_fini()
{
    lv_deinit();

    libgui_display_fini();

    kbd_indev = NULL;
    mouse_indev = NULL;
    cursor_obj = NULL;

    vinfo_xres = 0;
    vinfo_yres = 0;

    //// Mouse configs
    lines_per_wheel = 0;
}

void libgui_mouse_get_pos(uint16_t* out_pos_x, uint16_t* out_pos_y)
{
    lv_point_t p;

    if(!mouse_indev)
        return;

    lv_indev_get_point(mouse_indev, &p);

    if(out_pos_x)
        *out_pos_x = p.x;
    if(out_pos_y)
        *out_pos_y = p.y;
}

void libgui_cycle(void)
{
    uint32_t sleep_ms = lv_timer_handler();

    // usleep(sleep_ms * 1000);
}