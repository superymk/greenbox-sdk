// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
#include <sap_headers.h>
#include <palddk.h>

#include <rdesktop_header.h>

// #include "frm_blank.h"

extern void sp_kbd_listener(struct kbd_key_event *event);
extern void sp_mouse_listener(struct mouse_event *event);

extern lv_obj_t *ui_ScreenLogin;
extern lv_obj_t *ui_tMessage;
extern lv_obj_t *ui_tServerPWD;

extern lv_obj_t *ui_wnd_rdesktop;

/// @brief which form opens up the login form?
static lv_obj_t *_wnd_from = NULL;

/// @brief True iff the login window is called during the first <rfbInitClient>
static bool _wnd_login_is_first_load = true;

/// @brief Used by <wnd_login_show>
static bool _temp_wnd_login_keep_show = true;

/// @brief Destroy the login window.
static void wnd_login_destroy(void);

/// @brief Init the UI of the login window and show it.
/// @param error_msg 
static void _wnd_login_show()
{
    _wnd_from = wnd_get_current();

    lv_textarea_set_password_show_time(ui_tServerPWD, 0);
    // lv_label_set_text(ui_tMessage, error_msg);

    lv_disp_load_scr(ui_ScreenLogin);
}

/// @brief Destroy the login screen
static void _wnd_login_destroy(void)
{
    if (_wnd_from)
        wnd_switch_to(_wnd_from);
}

static void wnd_login_destroy(void)
{
    libdtm_hid_register_kbd_listener(sp_kbd_listener);
    libdtm_hid_register_mouse_listener(sp_mouse_listener);

    _temp_wnd_login_keep_show = false;
}

void OnClick_bLoginServer(lv_event_t *e)
{
    // SAP_STATUS status = SUCCESS;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        const char *password = lv_textarea_get_text(ui_tServerPWD);

        cred_password_set(password);

        wnd_login_destroy(); //_wnd_login_destroy();
    }
}

void wnd_login_unmark_first_load(void)
{
    _wnd_login_is_first_load = false;
}

void wnd_login_mark_first_load(void)
{
    _wnd_login_is_first_load = true;
}

// static void event_cb(lv_event_t * e)
// {
//     // lv_obj_t * obj = lv_event_get_current_target(e);
//     // LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));
//     // (void)obj;
// }

// void OnClick_BtnTerminate(lv_event_t * e)
// {
//     // // sp_dtm_terminate();
//     // const char * btns[] = {"Apply", "Close", ""};
//     // lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Info", "Please close the window to exit GreenBox.", btns, true);
//     // lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
//     // lv_obj_center(mbox1);
// }

void wnd_login_show(void)
{
    libdtm_hid_unregister_kbd_listener(sp_kbd_listener);
    libdtm_hid_unregister_mouse_listener(sp_mouse_listener);

    // Display the login window
    _temp_wnd_login_keep_show = true;
    if (_wnd_login_is_first_load)
    {
        // _wnd_login_show("");
        _wnd_login_show();
    }
    else
    {
        // _wnd_login_show("Incorrect Connection Password!");
        _wnd_login_show();
    }

    while (_temp_wnd_login_keep_show)
    {
        // Pull and handle all DesktopMgr events (e.g., key presses)
        libdtm_handle_dtm_events();

        // For GUI
        libgui_cycle();

        // cpu_relax();
        sp_yield();
    }
}

/*********** Temp Functions **********/
#define BLOCK_WAIT_MSLEEP_PER_ROUND (50)
void _temp_wait_in_wnd_login(const char *error_msg, uint32_t sleep_ms)
{
    volatile uint32_t timeout_round = sleep_ms / BLOCK_WAIT_MSLEEP_PER_ROUND;

    // Forward HID devices to LVGL
    libdtm_hid_unregister_kbd_listener(sp_kbd_listener);
    libdtm_hid_unregister_mouse_listener(sp_mouse_listener);

    // Show error message, if any.
    lv_label_set_text(ui_tMessage, error_msg);

    // Wait without pausing LVGL
    do
    {
        // Pull and handle all DesktopMgr events (e.g., key presses)
        libdtm_handle_dtm_events();

        // For GUI
        libgui_cycle();

        usleep(BLOCK_WAIT_MSLEEP_PER_ROUND * 1000);
        timeout_round--;
    } while (timeout_round != 0);

    return;
}