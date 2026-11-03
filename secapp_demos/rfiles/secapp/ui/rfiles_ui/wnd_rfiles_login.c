// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/************************************************************************************
 * Remote File Client — Login Window (LVGL v9.3+)
 ************************************************************************************/
#include "wnd_rfiles_login.h"
#include "wnd_rfiles_list.h"
#include <string.h>

/* Full teardown for Exit */
extern void wnd_rfiles_close(void);

/* Google Drive backend */
extern int gdrive_begin_device_code_flow_async(SAP_CONFIG* sap_cfg, gdrive_user_code_cb_t cb, void *user_ctx);
extern void gdrive_cancel_device_code_flow(void);

/* Token poller (single attempt, returns EAGAIN to continue) dispatched async by backend */
extern int gdrive_begin_token_poll_async(gdrive_login_done_cb_t cb, void *user_ctx);
extern void gdrive_cancel_token_poll(void);

static void _msgbox_show_info(const char *info_type, const char *str_err);
static void gdrive_token_done_cb(SAP_STATUS sap_status, void *user_ctx);

typedef enum
{
    PROTO_GDRIVE = 0,
    PROTO_SMB = 1
} protocol_t;

typedef struct
{
    lv_obj_t *root;

    /* bars + center */
    lv_obj_t *topbar;
    lv_obj_t *center_panel; /* flex-grow container */
    lv_obj_t *bottombar;
    lv_obj_t *dropdown;

    lv_obj_t *btn_close;
    lv_obj_t *btn_exit;

    /* GDrive */
    lv_obj_t *gd_container;
    lv_obj_t *gd_spinner;
    lv_obj_t *gd_label_status;
    lv_obj_t *gd_label_connecting_title;
#if LV_USE_QRCODE
    lv_obj_t *gd_qr;
#endif
    lv_obj_t *gd_label_code;
    lv_obj_t *gd_label_hint;

    /* “waiting for authorization” row under QR */
    lv_obj_t *gd_wait_row;
    lv_obj_t *gd_wait_spinner;
    lv_obj_t *gd_wait_label;

    /* Timer used to schedule the next single poll when backend returns EAGAIN */
    lv_timer_t *gd_poll_timer;

    uint32_t spinner_start_ms;
    char cached_user_code[64];
    char cached_ver_url[256];
    char cached_ver_url_with_code[512];

    /* SMB */
    lv_obj_t *smb_container;
    lv_obj_t *smb_tf_servaddr;
    lv_obj_t *smb_tf_username;
    lv_obj_t *smb_tf_password;
    lv_obj_t *smb_btn_login;

    /* callback */
    wnd_login_closed_cb_t on_closed;
    void *on_closed_ctx;
} login_ui_t;

static SAP_CONFIG* sap_cfg = NULL;

/* ------------- helpers ------------- */
static login_ui_t *get_ui(lv_obj_t *root)
{
    return (login_ui_t *)lv_obj_get_user_data(root);
}

static void set_ui(lv_obj_t *root, login_ui_t *ui)
{
    lv_obj_set_user_data(root, ui);
}

static void remove_all_styles(lv_obj_t *o)
{
    if (!o)
        return;
    lv_obj_remove_style_all(o);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_outline_width(o, 0, 0);
    lv_obj_set_style_shadow_width(o, 0, 0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
}
static void remove_all_children(lv_obj_t *obj)
{
    if (!obj)
        return;
    while (lv_obj_get_child_cnt(obj))
        lv_obj_delete(lv_obj_get_child(obj, 0));
}
static void apply_card_style(lv_obj_t *obj)
{
    lv_obj_set_style_radius(obj, 12, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_100, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xDADCE0), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_pad_all(obj, 12, 0); /* compact padding to fit 800x600 */
    lv_obj_set_style_shadow_width(obj, 8, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_20, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}
static void apply_title_style(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, lv_theme_get_font_large(label), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x202124), 0);
}
static void apply_subtitle_style(lv_obj_t *label)
{
    /* v9.3 has small/large – use small as “subtitle” to keep size hierarchy */
    lv_obj_set_style_text_font(label, lv_theme_get_font_small(label), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x202124), 0);
}
static void apply_hint_style(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, lv_theme_get_font_small(label), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x5F6368), 0);
}

static void make_textarea_one_line_(lv_obj_t *ta)
{
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(ta, LV_OBJ_FLAG_SCROLLABLE);
}

/* ------------- GDrive flow ------------- */
typedef struct
{
    login_ui_t *ui;
    uint32_t earliest_ms;
} gdrive_async_ctx_t;

static void gdrive_show_qr_now(login_ui_t *ui);

/* ---------- retry scheduling for single-poll EAGAIN ---------- */

static void _poll_again_timer_cb(lv_timer_t *tm)
{
    login_ui_t *ui = (login_ui_t *)lv_timer_get_user_data(tm);
    if (!ui)
        return;

    /* one-shot; clear handle */
    ui->gd_poll_timer = NULL;

    /* Fire one async poll attempt; backend will call gdrive_token_done_cb() */
    (void)gdrive_begin_token_poll_async(gdrive_token_done_cb, ui);
}

static void _schedule_poll_again(login_ui_t *ui, uint32_t delay_ms)
{
    if (!ui)
        return;

    /* replace any pending retry */
    if (ui->gd_poll_timer)
    {
        lv_timer_del(ui->gd_poll_timer);
        ui->gd_poll_timer = NULL;
    }

    ui->gd_poll_timer = lv_timer_create(_poll_again_timer_cb,
                                        delay_ms ? delay_ms : 50, /* default 50ms */
                                        ui);
    lv_timer_set_repeat_count(ui->gd_poll_timer, 1);
}

/* ---------- async QR replace after 1s minimum spinner ---------- */

static void gdrive_show_qr_timer_cb(lv_timer_t *t)
{
    login_ui_t *ui = (login_ui_t *)lv_timer_get_user_data(t);
    gdrive_show_qr_now(ui);
}

static void gdrive_async_gui_handler(void *p)
{
    gdrive_async_ctx_t *ctx = (gdrive_async_ctx_t *)p;
    if (!ctx || !ctx->ui)
    {
        lv_free(ctx);
        return;
    }

    uint32_t now = lv_tick_get();
    if (now < ctx->earliest_ms)
    {
        lv_timer_t *tm = lv_timer_create_basic();
        lv_timer_set_period(tm, (ctx->earliest_ms - now) ? (ctx->earliest_ms - now) : 1);
        lv_timer_set_repeat_count(tm, 1);
        lv_timer_set_user_data(tm, ctx->ui);
        lv_timer_set_cb(tm, gdrive_show_qr_timer_cb);
        lv_free(ctx);
        return;
    }

    gdrive_show_qr_now(ctx->ui);
    lv_free(ctx);
}

static void gdrive_backend_cb(const char *user_code,
                              const char *verification_url,
                              const char *verification_url_with_code,
                              void *user_ctx)
{
    login_ui_t *ui = (login_ui_t *)user_ctx;
    if (!ui)
        return;

    lv_snprintf(ui->cached_user_code, sizeof(ui->cached_user_code), "%s", user_code ? user_code : "");
    lv_snprintf(ui->cached_ver_url, sizeof(ui->cached_ver_url), "%s", verification_url ? verification_url : "");
    lv_snprintf(ui->cached_ver_url_with_code, sizeof(ui->cached_ver_url_with_code), "%s", verification_url_with_code ? verification_url_with_code : "");

    gdrive_async_ctx_t *ctx = (gdrive_async_ctx_t *)lv_malloc(sizeof(*ctx));
    ctx->ui = ui;
    ctx->earliest_ms = ui->spinner_start_ms + 1000;
    lv_async_call(gdrive_async_gui_handler, ctx);
}

static void gdrive_build_connecting(login_ui_t *ui)
{
    ui->gd_container = lv_obj_create(ui->center_panel);
    apply_card_style(ui->gd_container);
    lv_obj_set_size(ui->gd_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(ui->gd_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(ui->gd_container, 8, 0);
    lv_obj_center(ui->gd_container);

    ui->gd_label_connecting_title = lv_label_create(ui->gd_container);
    lv_label_set_text(ui->gd_label_connecting_title, "Connecting to Google");
    apply_subtitle_style(ui->gd_label_connecting_title);

    ui->gd_spinner = lv_spinner_create(ui->gd_container);
    lv_obj_set_size(ui->gd_spinner, 100, 100); /* smaller to fit 800×600 */
    lv_obj_center(ui->gd_spinner);

    ui->gd_label_status = lv_label_create(ui->gd_container);
    lv_label_set_text(ui->gd_label_status, "Please wait. This may take a moment.");
    apply_hint_style(ui->gd_label_status);

    ui->spinner_start_ms = lv_tick_get();
    ui->cached_user_code[0] = ui->cached_ver_url[0] = ui->cached_ver_url_with_code[0] = 0;

    if (gdrive_begin_device_code_flow_async(sap_cfg, gdrive_backend_cb, ui) != 0)
    {
        lv_label_set_text(ui->gd_label_status, "Failed to connect. Please try again.");
        apply_hint_style(ui->gd_label_status);
    }
}

struct done_pack
{
    login_ui_t *ui;
    SAP_STATUS st;
};

/* runs on UI thread */
static void _gdrive_token_done_ui(void *p)
{
    struct done_pack *dp = (struct done_pack *)p;
    if (!dp || !dp->ui)
    {
        if (dp)
            lv_free(dp);
        return;
    }
    login_ui_t *ui = dp->ui;
    SAP_STATUS st = dp->st;

    /* Soft-continue case: backend says "not yet" → schedule the next attempt */
    if (st == EAGAIN)
    {
        /* Keep spinner/label as-is; just retry soon (or choose a larger delay if you track next_poll_at) */
        _schedule_poll_again(ui, 50);
        lv_free(dp);
        return;
    }

    /* stop the “waiting” spinner, update text */
    if (ui->gd_wait_spinner)
    {
        lv_obj_delete(ui->gd_wait_spinner);
        ui->gd_wait_spinner = NULL;
    }

    if (st == SUCCESS /* SUCCESS */)
    {
        if (ui->gd_wait_label)
            lv_label_set_text(ui->gd_wait_label, "Authorized. Finishing…");

        // auto-close the login window
        wnd_rfiles_login_close(ui->root);

        // Show the file list page
        wnd_rfiles_list_show(wnd_rfiles_get_root());
    }
    else
    {
        if (ui->gd_wait_label)
            lv_label_set_text(ui->gd_wait_label, "Authorization failed. Please try again.");
        _msgbox_show_info("Google Drive", "Login failed. Please try again.");
    }

    lv_free(dp);
}

/* backend completion callback (may be called on NON-UI thread) */
static void gdrive_token_done_cb(SAP_STATUS sap_status, void *user_ctx)
{
    login_ui_t *ui = (login_ui_t *)user_ctx;
    if (!ui)
        return;

    struct done_pack *dp = lv_malloc(sizeof(*dp));
    if (!dp)
        return;
    dp->ui = ui;
    dp->st = sap_status;

    /* Ensure UI-thread execution */
    lv_async_call(_gdrive_token_done_ui, dp);
}

static void gdrive_show_qr_now(login_ui_t *ui)
{
    if (!ui || !ui->gd_container)
        return;

    if (ui->gd_label_connecting_title)
    {
        lv_obj_delete(ui->gd_label_connecting_title);
        ui->gd_label_connecting_title = NULL;
    }

    if (ui->gd_spinner)
    {
        lv_obj_delete(ui->gd_spinner);
        ui->gd_spinner = NULL;
    }

    if (ui->gd_label_status)
    {
        lv_label_set_text(ui->gd_label_status, "Scan QR code to login:");
        apply_subtitle_style(ui->gd_label_status);
    }

    lv_obj_t *row = lv_obj_create(ui->gd_container);
    remove_all_styles(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(row, 16, 0);
    lv_obj_set_flex_grow(row, 1);

#if LV_USE_QRCODE
    ui->gd_qr = lv_qrcode_create(row);
    lv_qrcode_set_size(ui->gd_qr, 180); /* smaller to guarantee fit at 800×600 */
    lv_qrcode_set_dark_color(ui->gd_qr, lv_color_black());
    lv_qrcode_set_light_color(ui->gd_qr, lv_color_white());
#endif

    const char *qr_data = ui->cached_ver_url_with_code[0] ? ui->cached_ver_url_with_code : (ui->cached_ver_url[0] ? ui->cached_ver_url : "https://www.google.com/device");
#if LV_USE_QRCODE
    lv_qrcode_update(ui->gd_qr, qr_data, strlen(qr_data));
#endif

    lv_obj_t *col = lv_obj_create(row);
    remove_all_styles(col);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 6, 0);
    lv_obj_set_flex_grow(col, 1);
    lv_obj_set_width(col, LV_PCT(100));

    ui->gd_label_code = lv_label_create(col);
    lv_label_set_text_fmt(ui->gd_label_code, "%s",
                          ui->cached_user_code[0] ? ui->cached_user_code : "—");
#if LV_FONT_MONTSERRAT_28
    lv_obj_set_style_text_font(ui->gd_label_code, &lv_font_montserrat_28, 0);
#else
    lv_obj_set_style_text_font(ui->gd_label_code, lv_theme_get_font_large(ui->gd_label_code), 0);
#endif
    lv_obj_set_style_text_color(ui->gd_label_code, lv_color_hex(0x1A73E8), 0);

    ui->gd_label_hint = lv_label_create(col);
    lv_label_set_long_mode(ui->gd_label_hint, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ui->gd_label_hint, LV_PCT(100));
    lv_label_set_text_fmt(ui->gd_label_hint, "Or open:\n%s\nand enter the code above.",
                          ui->cached_ver_url[0] ? ui->cached_ver_url : "https://www.google.com/device");
    apply_hint_style(ui->gd_label_hint);

    /* --- New line: “Waiting for authorization on your phone …” with a small spinner --- */
    ui->gd_wait_row = lv_obj_create(ui->gd_container);
    remove_all_styles(ui->gd_wait_row);
    lv_obj_set_width(ui->gd_wait_row, LV_PCT(100));
    lv_obj_set_flex_flow(ui->gd_wait_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(ui->gd_wait_row, 8, 0);
    lv_obj_set_style_pad_top(ui->gd_wait_row, 4, 0);

    /* spinner ~= text size (use theme small font height as a proxy) */
    ui->gd_wait_spinner = lv_spinner_create(ui->gd_wait_row);
    int sp = 0;
    const lv_font_t *fsmall = lv_theme_get_font_small(ui->gd_wait_row);
    if (fsmall)
        sp = fsmall->line_height;
    if (sp <= 0)
        sp = 18; /* fallback */
    lv_obj_set_size(ui->gd_wait_spinner, sp, sp);

    ui->gd_wait_label = lv_label_create(ui->gd_wait_row);
    lv_label_set_text(ui->gd_wait_label, "Waiting for authorization on your phone...");
    apply_hint_style(ui->gd_wait_label);

    /* Kick off the first single poll asynchronously (non-blocking).
       On EAGAIN, the UI callback schedules the next attempt via a timer. */
    if (gdrive_begin_token_poll_async(/* cb */ gdrive_token_done_cb, /* user_ctx */ ui) != 0)
    {
        /* If we cannot start the poll, show an error immediately */
        lv_label_set_text(ui->gd_wait_label, "Failed to start authorization polling. Please try again.");
    }
}

/* ------------- SMB view ------------- */
static void _msgbox_show_info(const char *info_type, const char *str_err)
{
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mbox, info_type);
    lv_msgbox_add_text(mbox, str_err);
    lv_msgbox_add_close_button(mbox);
    lv_obj_center(mbox);
}

static void on_smb_login_clicked(lv_event_t *e)
{
    (void)e;
    _msgbox_show_info("SMB", "Login clicked (wire backend)");
}

static void build_smb_view(login_ui_t *ui)
{
    if (ui->gd_container)
    {
        lv_obj_delete(ui->gd_container);
        ui->gd_container = NULL;
    }
    /* cancel any pending poll timer when switching views */
    if (ui->gd_poll_timer)
    {
        lv_timer_del(ui->gd_poll_timer);
        ui->gd_poll_timer = NULL;
    }

    remove_all_children(ui->center_panel);

    ui->smb_container = lv_obj_create(ui->center_panel);
    apply_card_style(ui->smb_container);
    lv_obj_set_size(ui->smb_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(ui->smb_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(ui->smb_container, 8, 0);

    lv_obj_t *title = lv_label_create(ui->smb_container);
    lv_label_set_text(title, "Sign in to SMB");
    apply_subtitle_style(title);

    ui->smb_tf_servaddr = lv_textarea_create(ui->smb_container);
    lv_textarea_set_placeholder_text(ui->smb_tf_servaddr, "Server Address (e.g., 192.168.1.20)");
    lv_obj_set_width(ui->smb_tf_servaddr, LV_PCT(60));
    make_textarea_one_line_(ui->smb_tf_servaddr);

    ui->smb_tf_username = lv_textarea_create(ui->smb_container);
    lv_textarea_set_placeholder_text(ui->smb_tf_username, "Domain\\Username (e.g., WORKSTATION\\Superymk)");
    lv_obj_set_width(ui->smb_tf_username, LV_PCT(60));
    make_textarea_one_line_(ui->smb_tf_username);

    ui->smb_tf_password = lv_textarea_create(ui->smb_container);
    lv_textarea_set_password_mode(ui->smb_tf_password, true);
    lv_textarea_set_password_show_time(ui->smb_tf_password, 0);
    lv_textarea_set_placeholder_text(ui->smb_tf_password, "Password");
    lv_obj_set_width(ui->smb_tf_password, LV_PCT(60));
    make_textarea_one_line_(ui->smb_tf_password);

    ui->smb_btn_login = lv_btn_create(ui->smb_container);
    lv_obj_add_event_cb(ui->smb_btn_login, on_smb_login_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl = lv_label_create(ui->smb_btn_login);
    lv_label_set_text(lbl, "Login");
    lv_obj_center(lbl);
}

/* ------------- protocol switching ------------- */
static void build_center_for_protocol(login_ui_t *ui, protocol_t p)
{
    if (p == PROTO_GDRIVE)
        gdrive_build_connecting(ui);
    else if (p == PROTO_SMB)
        build_smb_view(ui);
}

static void on_dropdown_changed(lv_event_t *e)
{
    login_ui_t *ui = (login_ui_t *)lv_event_get_user_data(e);
    uint16_t sel = lv_dropdown_get_selected(ui->dropdown);

    // stop any ongoing backend work when switching away
    if (ui->gd_poll_timer)
    {
        lv_timer_del(ui->gd_poll_timer);
        ui->gd_poll_timer = NULL;
    }
    gdrive_cancel_device_code_flow();
    gdrive_cancel_token_poll();

    if (sel == 0)
    {
        // blank --> clear login area
        remove_all_children(ui->center_panel);
        return;
    }

    protocol_t p = (sel == 1) ? PROTO_GDRIVE : PROTO_SMB;
    build_center_for_protocol(ui, p);
}

/* ------------- bottom actions ------------- */
static void on_btn_close(lv_event_t *e)
{
    login_ui_t *ui = (login_ui_t *)lv_event_get_user_data(e);
    if (!ui)
        return;

    if (ui->gd_poll_timer)
    {
        lv_timer_del(ui->gd_poll_timer);
        ui->gd_poll_timer = NULL;
    }
    gdrive_cancel_device_code_flow();
    gdrive_cancel_token_poll();

    wnd_login_closed_cb_t cb = ui->on_closed;
    void *ctx = ui->on_closed_ctx;
    lv_obj_delete_delayed(ui->root, 0);
    if (cb)
        cb(ctx);
}

static void on_btn_exit(lv_event_t *e)
{
    login_ui_t *ui = (login_ui_t *)lv_event_get_user_data(e);
    if (ui && ui->gd_poll_timer)
    {
        lv_timer_del(ui->gd_poll_timer);
        ui->gd_poll_timer = NULL;
    }

    gdrive_cancel_device_code_flow();
    gdrive_cancel_token_poll();

    wnd_rfiles_close();
}

/* ------------- public API ------------- */
lv_obj_t *wnd_rfiles_login_create(lv_obj_t *parent,
                                  wnd_login_closed_cb_t on_closed,
                                  void *user_ctx)
{
    login_ui_t *ui = (login_ui_t *)lv_malloc(sizeof(login_ui_t));
    LV_ASSERT(ui);
    memset(ui, 0, sizeof(*ui));
    ui->on_closed = on_closed;
    ui->on_closed_ctx = user_ctx;

    /* Root (column) fills parent; no scroll; white bg by inheritance */
    ui->root = lv_obj_create(parent);
    remove_all_styles(ui->root);
    lv_obj_set_size(ui->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(ui->root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(ui->root, 8, 0);
    lv_obj_set_style_pad_row(ui->root, 8, 0);
    set_ui(ui->root, ui);

    /* Top bar: larger “Protocol:” to ensure visibility */
    ui->topbar = lv_obj_create(ui->root);
    remove_all_styles(ui->topbar);
    lv_obj_set_size(ui->topbar, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ui->topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(ui->topbar, 8, 0);
    lv_obj_set_style_align(ui->topbar, LV_ALIGN_LEFT_MID, 0);

    // keep items on one line; vertically center them
    lv_obj_set_flex_align(ui->topbar,
                          LV_FLEX_ALIGN_START,   // main axis
                          LV_FLEX_ALIGN_CENTER,  // cross axis
                          LV_FLEX_ALIGN_CENTER); // track cross

    lv_obj_t *proto_lbl = lv_label_create(ui->topbar);
    lv_label_set_text(proto_lbl, "Protocol:");
    /* make it clearly visible */
    lv_obj_set_style_text_font(proto_lbl, lv_theme_get_font_large(proto_lbl), 0);
    lv_obj_set_style_text_color(proto_lbl, lv_color_hex(0x202124), 0);

    ui->dropdown = lv_dropdown_create(ui->topbar);
    lv_dropdown_set_options(ui->dropdown,
                            "\n" // <-- blank first row
                            "Google Drive\n"
                            "SMB (coming soon)");
    lv_dropdown_set_selected(ui->dropdown, 0);
    lv_obj_add_event_cb(ui->dropdown, on_dropdown_changed, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_set_width(ui->dropdown, 240);            /* ≈18–20 chars */
    lv_obj_set_style_min_width(ui->dropdown, 0, 0); // permit shrink if space is tight
    /* keep the topbar compact */
    // (no extra styling needed)

    /* Center panel flexes to fill remaining vertical space */
    ui->center_panel = lv_obj_create(ui->root);
    remove_all_styles(ui->center_panel);
    lv_obj_set_size(ui->center_panel, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(ui->center_panel, 1);

    /* Bottom bar pinned at bottom, use GRID (8 equal columns) */
    ui->bottombar = lv_obj_create(ui->root);
    remove_all_styles(ui->bottombar);
    lv_obj_set_size(ui->bottombar, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_min_height(ui->bottombar, 40, 0);

    /* Switch layout to grid */
    static int cols[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST};
    static int rows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_layout(ui->bottombar, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(ui->bottombar, cols, rows);
    lv_obj_set_style_pad_left(ui->bottombar, 8, 0);
    lv_obj_set_style_pad_right(ui->bottombar, 8, 0);
    lv_obj_set_style_pad_top(ui->bottombar, 4, 0);
    lv_obj_set_style_pad_bottom(ui->bottombar, 4, 0);

    /* Close @ 25%: span 2 cols starting at col 1 (center of span = 25%) */
    ui->btn_close = lv_btn_create(ui->bottombar);
    lv_obj_add_event_cb(ui->btn_close, on_btn_close, LV_EVENT_CLICKED, ui);
    lv_obj_set_grid_cell(ui->btn_close,
                         LV_GRID_ALIGN_CENTER, /* horizontal align */
                         1,                    /* col start */
                         2,                    /* col span */
                         LV_GRID_ALIGN_CENTER, /* vertical align */
                         0,                    /* row start */
                         1);                   /* row span */
    lv_obj_t *lbl_close = lv_label_create(ui->btn_close);
    lv_label_set_text(lbl_close, "Close");
    lv_obj_center(lbl_close);

    /* Exit @ 75%: span 2 cols starting at col 5 (center of span = 75%) */
    ui->btn_exit = lv_btn_create(ui->bottombar);
    lv_obj_add_event_cb(ui->btn_exit, on_btn_exit, LV_EVENT_CLICKED, ui);
    lv_obj_set_grid_cell(ui->btn_exit,
                         LV_GRID_ALIGN_CENTER,
                         5, 2,
                         LV_GRID_ALIGN_CENTER,
                         0, 1);
    lv_obj_t *lbl_exit = lv_label_create(ui->btn_exit);
    lv_label_set_text(lbl_exit, "Exit");
    lv_obj_center(lbl_exit);

    return ui->root;
}

void wnd_rfiles_login_close(lv_obj_t *login_root)
{
    if (!login_root)
        return;
    login_ui_t *ui = get_ui(login_root);
    if (!ui)
    {
        lv_obj_delete(login_root);
        return;
    }

    if (ui->gd_poll_timer)
    {
        lv_timer_del(ui->gd_poll_timer);
        ui->gd_poll_timer = NULL;
    }
    gdrive_cancel_device_code_flow();
    gdrive_cancel_token_poll();

    wnd_login_closed_cb_t cb = ui->on_closed;
    void *ctx = ui->on_closed_ctx;

    lv_obj_delete_delayed(login_root, 0);
    if (cb)
        cb(ctx);
}

bool wnd_rfiles_login_is_root(lv_obj_t *obj)
{
    login_ui_t *ui = get_ui(obj);
    return ui && ui->root == obj;
}

SAP_STATUS wnd_rfiles_login_init_cfgs(SAP_CONFIG *in_sap_cfg)
{
    sap_cfg = in_sap_cfg;

    // On success
    return SUCCESS;
}