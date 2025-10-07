// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/uinput.h>

// #include <appddk.h>
#include <ipc/varbuf.h>
#include "secapp/app_sp_bridge_cmds.h"
#include "hid_bridge/hid_bridge_defs.h"
#include "hid_bridge/hid_ops.h"

#define DEFAULT_FD_KBD (-1)   // Note: We do not set <fd> to 0 because 0 is a valid <fd>
#define DEFAULT_FD_MOUSE (-1) // Note: We do not set <fd> to 0 because 0 is a valid <fd>

static uint32_t libdrv_hid_to_hid_map[256] =
    {
        [LIBDRV_HID_KEY_ESC] KEY_ESC,
        [LIBDRV_HID_KEY_1] KEY_1,
        [LIBDRV_HID_KEY_2] KEY_2,
        [LIBDRV_HID_KEY_3] KEY_3,
        [LIBDRV_HID_KEY_4] KEY_4,
        [LIBDRV_HID_KEY_5] KEY_5,
        [LIBDRV_HID_KEY_6] KEY_6,
        [LIBDRV_HID_KEY_7] KEY_7,
        [LIBDRV_HID_KEY_8] KEY_8,
        [LIBDRV_HID_KEY_9] KEY_9,
        [LIBDRV_HID_KEY_0] KEY_0,
        [LIBDRV_HID_KEY_MINUS] KEY_MINUS,
        [LIBDRV_HID_KEY_EQUAL] KEY_EQUAL,
        [LIBDRV_HID_KEY_BACKSPACE] KEY_BACKSPACE,
        [LIBDRV_HID_KEY_TAB] KEY_TAB,
        [LIBDRV_HID_KEY_Q] KEY_Q,
        [LIBDRV_HID_KEY_W] KEY_W,
        [LIBDRV_HID_KEY_E] KEY_E,
        [LIBDRV_HID_KEY_R] KEY_R,
        [LIBDRV_HID_KEY_T] KEY_T,
        [LIBDRV_HID_KEY_Y] KEY_Y,
        [LIBDRV_HID_KEY_U] KEY_U,
        [LIBDRV_HID_KEY_I] KEY_I,
        [LIBDRV_HID_KEY_O] KEY_O,
        [LIBDRV_HID_KEY_P] KEY_P,
        [LIBDRV_HID_KEY_LEFT_SQUARE] KEY_LEFTBRACE,
        [LIBDRV_HID_KEY_RIGHT_SQUARE] KEY_RIGHTBRACE,
        [LIBDRV_HID_KEY_ENTER] KEY_ENTER,
        [LIBDRV_HID_KEY_LEFT_CTRL] KEY_LEFTCTRL,
        [LIBDRV_HID_KEY_A] KEY_A,
        [LIBDRV_HID_KEY_S] KEY_S,
        [LIBDRV_HID_KEY_D] KEY_D,
        [LIBDRV_HID_KEY_F] KEY_F,
        [LIBDRV_HID_KEY_G] KEY_G,
        [LIBDRV_HID_KEY_H] KEY_H,
        [LIBDRV_HID_KEY_J] KEY_J,
        [LIBDRV_HID_KEY_K] KEY_K,
        [LIBDRV_HID_KEY_L] KEY_L,
        [LIBDRV_HID_KEY_SEMICOLON] KEY_SEMICOLON,
        [LIBDRV_HID_KEY_QUOTE] KEY_APOSTROPHE,
        [LIBDRV_HID_KEY_BACKTICK] KEY_GRAVE,
        [LIBDRV_HID_KEY_LEFT_SHIFT] KEY_LEFTSHIFT,
        [LIBDRV_HID_KEY_BACKSLASH] KEY_BACKSLASH,
        [LIBDRV_HID_KEY_Z] KEY_Z,
        [LIBDRV_HID_KEY_X] KEY_X,
        [LIBDRV_HID_KEY_C] KEY_C,
        [LIBDRV_HID_KEY_V] KEY_V,
        [LIBDRV_HID_KEY_B] KEY_B,
        [LIBDRV_HID_KEY_N] KEY_N,
        [LIBDRV_HID_KEY_M] KEY_M,
        [LIBDRV_HID_KEY_COMMA] KEY_COMMA,
        [LIBDRV_HID_KEY_PERIOD] KEY_DOT,
        [LIBDRV_HID_KEY_SLASH] KEY_SLASH,
        [LIBDRV_HID_KEY_RIGHT_SHIFT] KEY_RIGHTSHIFT,
        [LIBDRV_HID_KEYPAD_ASTERIX] KEY_KPASTERISK,
        [LIBDRV_HID_KEY_LEFT_ALT] KEY_LEFTALT,
        [LIBDRV_HID_KEY_SPACE] KEY_SPACE,
        [LIBDRV_HID_KEY_CAPS_LOCK] KEY_CAPSLOCK,
        [LIBDRV_HID_KEY_F1] KEY_F1,
        [LIBDRV_HID_KEY_F2] KEY_F2,
        [LIBDRV_HID_KEY_F3] KEY_F3,
        [LIBDRV_HID_KEY_F4] KEY_F4,
        [LIBDRV_HID_KEY_F5] KEY_F5,
        [LIBDRV_HID_KEY_F6] KEY_F6,
        [LIBDRV_HID_KEY_F7] KEY_F7,
        [LIBDRV_HID_KEY_F8] KEY_F8,
        [LIBDRV_HID_KEY_F9] KEY_F9,
        [LIBDRV_HID_KEY_F10] KEY_F10,
        [LIBDRV_HID_KEY_NUM_LOCK] KEY_NUMLOCK,
        [LIBDRV_HID_KEY_SCROLL_LOCK] KEY_SCROLLLOCK,
        [LIBDRV_HID_KEYPAD_7] KEY_KP7,
        [LIBDRV_HID_KEYPAD_8] KEY_KP8,
        [LIBDRV_HID_KEYPAD_9] KEY_KP9,
        [LIBDRV_HID_KEYPAD_MINUS] KEY_KPMINUS,
        [LIBDRV_HID_KEYPAD_4] KEY_KP4,
        [LIBDRV_HID_KEYPAD_5] KEY_KP5,
        [LIBDRV_HID_KEYPAD_6] KEY_KP6,
        [LIBDRV_HID_KEYPAD_PLUS] KEY_KPPLUS,
        [LIBDRV_HID_KEYPAD_1] KEY_KP1,
        [LIBDRV_HID_KEYPAD_2] KEY_KP2,
        [LIBDRV_HID_KEYPAD_3] KEY_KP3,
        [LIBDRV_HID_KEYPAD_0] KEY_KP0,
        [LIBDRV_HID_KEYPAD_PERIOD] KEY_KPDOT,
        [LIBDRV_HID_KEY_F11] KEY_F11,
        [LIBDRV_HID_KEY_F12] KEY_F12,
        [LIBDRV_HID_KEY_PRINTSCREEN] KEY_SYSRQ,
        [LIBDRV_HID_KEYPAD_ENTER] KEY_KPENTER,
        [LIBDRV_HID_KEY_RIGHT_CTRL] KEY_RIGHTCTRL,
        [LIBDRV_HID_KEYPAD_SLASH] KEY_KPSLASH,
        [LIBDRV_HID_KEY_RIGHT_ALT] KEY_RIGHTALT,
        [LIBDRV_HID_KEY_HOME] KEY_HOME,
        [LIBDRV_HID_KEY_UP] KEY_UP,
        [LIBDRV_HID_KEY_PAGE_UP] KEY_PAGEUP,
        [LIBDRV_HID_KEY_LEFT] KEY_LEFT,
        [LIBDRV_HID_KEY_RIGHT] KEY_RIGHT,
        [LIBDRV_HID_KEY_END] KEY_END,
        [LIBDRV_HID_KEY_DOWN] KEY_DOWN,
        [LIBDRV_HID_KEY_PAGE_DOWN] KEY_PAGEDOWN,
        [LIBDRV_HID_KEY_INSERT] KEY_INSERT,
        [LIBDRV_HID_KEY_DELETE] KEY_DELETE,
        [LIBDRV_HID_KEY_LEFT_GUI] KEY_LEFTMETA,
        [LIBDRV_HID_KEY_RIGHT_GUI] KEY_RIGHTMETA};

static int fd_kbd = DEFAULT_FD_KBD;
static int fd_mouse = DEFAULT_FD_MOUSE;

static int _hid_bridge_open_uinput(void)
{
    int fd = 0;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
        if (fd < 0)
        {
            HID_BRIDGE_ERR("Failed to open uinput: %s!\n", strerror(errno));
            return fd;
        }
    }

    return fd;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)

static bool _hid_bridge_init_kbd(int *out_fd)
{
    struct uinput_setup uud;
    int fd = 0;

    // Sanity checks
    if (!out_fd)
        return false;

    // Open uinput
    fd = _hid_bridge_open_uinput();
    if (fd < 0)
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Keyboard Error: Cannot find uinput!\n");
        return false;
    }

    // Enable all key buttons
    // [NOTE] Use <KEY_ALS_TOGGLE> instead of <KEY_CNT>, because uinput doesn't register device node anymore if too many
    // keys are set, see https://bugzilla.kernel.org/show_bug.cgi?id=118861#c1
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < KEY_ALS_TOGGLE; i++)
    {
        ioctl(fd, UI_SET_KEYBIT, i);
    }

    // // Enable LEDs
    ioctl(fd, UI_SET_EVBIT, EV_LED);
    for (int i = 0; i < LED_CNT; i++)
        ioctl(fd, UI_SET_LEDBIT, i);

    // // Enable sychronization
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Create the device
    memset(&uud, 0, sizeof(uud));
    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "Bridge-virtual-kbd");
    ioctl(fd, UI_DEV_SETUP, &uud);

    if (ioctl(fd, UI_DEV_CREATE))
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Keyboard Error: Failed to create the keyboard in uinput: %s!\n", strerror(errno));
        goto kbd_create_err;
    }

    // On success
    *out_fd = fd;
    return true;

kbd_create_err:
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return false;
}

static bool _hid_bridge_init_mouse(int *out_fd)
{
    struct uinput_setup uud;
    int fd = 0;

    // Sanity checks
    if (!out_fd)
        return false;

    // Open uinput
    fd = _hid_bridge_open_uinput();
    if (fd < 0)
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Mouse Error: Cannot find uinput!\n");
        return false;
    }

    // Enable all key buttons
    // [NOTE] Use <KEY_ALS_TOGGLE> instead of <KEY_CNT>, because uinput doesn't register device node anymore if too many
    // keys are set, see https://bugzilla.kernel.org/show_bug.cgi?id=118861#c1
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < KEY_ALS_TOGGLE; i++)
        ioctl(fd, UI_SET_KEYBIT, i);

    // Enable mouse axes
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    for (int i = 0; i < REL_CNT; i++)
        ioctl(fd, UI_SET_RELBIT, i);

    // Enable sychronization
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Create the device
    memset(&uud, 0, sizeof(uud));
    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "Bridge-virtual-mouse");
    ioctl(fd, UI_DEV_SETUP, &uud);

    if (ioctl(fd, UI_DEV_CREATE))
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Mouse Error: Failed to create the mouse in uinput: %s!\n", strerror(errno));
        goto mouse_create_err;
    }

    // On success
    *out_fd = fd;
    return true;

mouse_create_err:
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return false;
}
#else
static bool _hid_bridge_init_kbd(int *out_fd)
{
    struct uinput_user_dev uud;
    int fd = 0;

    // Sanity checks
    if (!out_fd)
        return false;

    // Open uinput
    fd = _hid_bridge_open_uinput();
    if (fd < 0)
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Keyboard Error: Cannot find uinput!\n");
        return false;
    }

    // Enable all key buttons
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < KEY_CNT; i++)
        ioctl(fd, UI_SET_KEYBIT, i);

    // Enable LEDs
    ioctl(fd, UI_SET_EVBIT, EV_LED);
    for (int i = 0; i < LED_CNT; i++)
        ioctl(fd, UI_SET_LEDBIT, i);

    // Enable sychronization
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Create the device
    memset(&uud, 0, sizeof(uud));
    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "Bridge-virtual-kbd");
    if (write(fd, &uud, sizeof(uud)) <= 0)
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Keyboard Error: Failed to register the keyboard!\n");
        goto kbd_register_err;
    }

    if (ioctl(fd, UI_DEV_CREATE))
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Keyboard Error: Failed to create the keyboard in uinput: %s!\n", strerror(errno));
        goto kbd_create_err;
    }

    // On success
    *out_fd = fd;
    return true;

kbd_create_err:
    ioctl(fd, UI_DEV_DESTROY);
kbd_register_err:
    close(fd);
    return false;
}

static bool _hid_bridge_init_mouse(int *out_fd)
{
    struct uinput_user_dev uud;
    int fd = 0;

    // Sanity checks
    if (!out_fd)
        return false;

    // Open uinput
    fd = _hid_bridge_open_uinput();
    if (fd < 0)
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Mouse Error: Cannot find uinput!\n");
        return false;
    }

    // Enable all key buttons
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < KEY_CNT; i++)
        ioctl(fd, UI_SET_KEYBIT, i);

    // Enable mouse axes
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    for (int i = 0; i < REL_CNT; i++)
        ioctl(fd, UI_SET_RELBIT, i);

    // Enable sychronization
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Create the device
    memset(&uud, 0, sizeof(uud));
    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "Bridge-virtual-mouse");
    if (write(fd, &uud, sizeof(uud)) <= 0)
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Mouse Error: Failed to register the mouse!\n");
        goto mouse_register_err;
    }

    if (ioctl(fd, UI_DEV_CREATE))
    {
        HID_BRIDGE_ERR("Create Bridge-Virtual Mouse Error: Failed to create the mouse in uinput: %s!\n", strerror(errno));
        goto mouse_create_err;
    }

    // On success
    *out_fd = fd;
    return true;

mouse_create_err:
    ioctl(fd, UI_DEV_DESTROY);
mouse_register_err:
    close(fd);
    return false;
}
#endif

static void _hid_bridge_fini_kbd()
{
    if (fd_kbd >= 0)
    {
        ioctl(fd_kbd, UI_DEV_DESTROY);
        close(fd_kbd);

        fd_kbd = DEFAULT_FD_KBD;
    }
}

static void _hid_bridge_fini_mouse(void)
{
    if (fd_mouse >= 0)
    {
        ioctl(fd_mouse, UI_DEV_DESTROY);
        close(fd_mouse);

        fd_mouse = DEFAULT_FD_MOUSE;
    }
}

static void emit(int fd, int type, int code, int val)
{
    struct input_event ie;
    ssize_t write_size = 0;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    // timestamp values below are ignored
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    errno = 0; // Clear errno before the call
    write_size = write(fd, &ie, sizeof(ie));
    if (write_size == -1)
    {
        HID_BRIDGE_ERR("Bridge-Virtual Keyboard and Mouse Error: Emit error (%s)!\n", strerror(errno));
    }
}

/********* Public Functions *********/
bool os_hid_bridge_init(void)
{
    bool status = false;

    status = _hid_bridge_init_kbd(&fd_kbd);
    if (!status)
    {
        HID_BRIDGE_ERR("Init Bridge-Virtual Keyboard Error!\n");
        goto init_kbd_err;
    }

    status = _hid_bridge_init_mouse(&fd_mouse);
    if (!status)
    {
        HID_BRIDGE_ERR("Init Bridge-Virtual Keyboard Error!\n");
        goto init_mouse_err;
    }

    // On success
    HID_BRIDGE_INFO("Init Bridge-Virtual Keyboard and Mouse Success\n");
    return true;

init_mouse_err:
    _hid_bridge_fini_kbd();
init_kbd_err:
    return false;
}

void os_hid_bridge_fini(void)
{
    _hid_bridge_fini_kbd();
    _hid_bridge_fini_mouse();
}

static void hid_bridge_kbd_press(int code)
{
    emit(fd_kbd, EV_KEY, code, 1);
    emit(fd_kbd, EV_SYN, SYN_REPORT, 0);
}

static void hid_bridge_kbd_release(int code)
{
    emit(fd_kbd, EV_KEY, code, 0);
    emit(fd_kbd, EV_SYN, SYN_REPORT, 0);
}

static void hid_bridge_mouse_press(int code)
{
    emit(fd_mouse, EV_KEY, code, 1);
    emit(fd_mouse, EV_SYN, SYN_REPORT, 0);
}

static void hid_bridge_mouse_release(int code)
{
    emit(fd_mouse, EV_KEY, code, 0);
    emit(fd_mouse, EV_SYN, SYN_REPORT, 0);
}

static void hid_bridge_mouse_move(int dx, int dy)
{
    emit(fd_mouse, EV_REL, REL_X, dx);
    emit(fd_mouse, EV_REL, REL_Y, dy);
    emit(fd_mouse, EV_SYN, SYN_REPORT, 0);
}

static void hid_bridge_mouse_wheel(int d)
{
    emit(fd_mouse, EV_REL, REL_WHEEL, d);
    emit(fd_mouse, EV_SYN, SYN_REPORT, 0);
}

static void hid_bridge_mouse_hwheel(int d)
{
    emit(fd_mouse, EV_REL, REL_HWHEEL, d);
    emit(fd_mouse, EV_SYN, SYN_REPORT, 0);
}

static void _hid_bridge_handle_kbd_event(struct kbd_key_event *event)
{
    if (!event)
        return;

    if (event->action != LIBDRV_HID_KBD_NO_EVENT)
    {
        int linux_hid_code = libdrv_hid_to_hid_map[event->key];

        if (event->action == LIBDRV_HID_KBD_KEY_PRESSED)
        {
            if (event->modifier)
                hid_bridge_kbd_press(linux_hid_code);
            else
                hid_bridge_kbd_press(linux_hid_code);
        }
        else if (event->action == LIBDRV_HID_KBD_KEY_RELEASED)
            hid_bridge_kbd_release(linux_hid_code);
    }
}

// static
void _hid_bridge_handle_mouse_event(struct mouse_event *event)
{
    if (!event)
        return;

    if (event->action != LIBDRV_HID_MOUSE_NO_EVENT)
    {
        if (event->action & LIBDRV_HID_MOUSE_BTN_PRESSED)
        {
            if (event->button & LIBDRV_HID_BTN_LEFT)
                hid_bridge_mouse_press(BTN_LEFT);
            if (event->button & LIBDRV_HID_BTN_RIGHT)
                hid_bridge_mouse_press(BTN_RIGHT);
            if (event->button & LIBDRV_HID_BTN_MIDDLE)
                hid_bridge_mouse_press(BTN_MIDDLE);
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

        if (event->action & LIBDRV_HID_MOUSE_BTN_RELEASED)
        {
            if (event->button & LIBDRV_HID_BTN_LEFT)
                hid_bridge_mouse_release(BTN_LEFT);
            if (event->button & LIBDRV_HID_BTN_RIGHT)
                hid_bridge_mouse_release(BTN_RIGHT);
            if (event->button & LIBDRV_HID_BTN_MIDDLE)
                hid_bridge_mouse_release(BTN_MIDDLE);
        }

        if (event->action == LIBDRV_HID_MOUSE_MOVED)
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
