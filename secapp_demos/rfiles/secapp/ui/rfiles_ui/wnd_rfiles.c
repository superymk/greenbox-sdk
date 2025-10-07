// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/************************************************************************************
 * Remote File Client — Main Window (LVGL v9.3+)
 ************************************************************************************/
#include "wnd_rfiles.h"
#include "wnd_rfiles_login.h"
#include <string.h>

typedef struct
{
    lv_obj_t *screen;     /* top-level screen we load */
    lv_obj_t *content;    /* parent for sub-windows */
    lv_obj_t *login_root; /* current login window root (if any) */
} wnd_rfiles_ctx_t;

static wnd_rfiles_ctx_t g_rfiles;
static bool g_finalizing = false;

/* -------------------- helpers -------------------- */
static void remove_all_styles(lv_obj_t *o, bool keep_bg)
{
    if (!o)
        return;

    if (!keep_bg)
    {
        lv_obj_remove_style_all(o);
        lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    }
    else
    {
        // don't remove everything; just clear what you need
        lv_obj_set_style_border_width(o, 0, 0);
        lv_obj_set_style_outline_width(o, 0, 0);
        lv_obj_set_style_shadow_width(o, 0, 0);
        // (leave bg color/opa intact)
    }

    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
}

static void apply_banner_style(lv_obj_t *banner)
{
    /* Reduced height ~50%: smaller paddings + smaller icon used in banner_build */
    lv_obj_set_style_radius(banner, 16, 0);
    lv_obj_set_style_pad_left(banner, 14, 0);
    lv_obj_set_style_pad_right(banner, 14, 0);
    lv_obj_set_style_pad_top(banner, 8, 0);
    lv_obj_set_style_pad_bottom(banner, 8, 0);
    lv_obj_set_style_min_height(banner, 56, 0); /* ~half of previous */

    lv_obj_set_style_bg_opa(banner, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(banner, lv_color_hex(0xE8F0FE), 0);

    lv_obj_set_style_shadow_width(banner, 8, 0);
    lv_obj_set_style_shadow_opa(banner, LV_OPA_20, 0);
}

static void banner_build(lv_obj_t *parent)
{
    lv_obj_t *row = lv_obj_create(parent);
    remove_all_styles(row, false);
    apply_banner_style(row);
    // lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_size(row, LV_PCT(100), 100);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(row, 10, 0);
    lv_obj_set_style_align(row, LV_ALIGN_CENTER, 0);

    /* Left: compact circular icon */
    lv_obj_t *icon_wrap = lv_obj_create(row);
    remove_all_styles(icon_wrap, false);
    lv_obj_set_size(icon_wrap, 40, 40); /* smaller icon => shorter banner */
    lv_obj_set_style_radius(icon_wrap, 999, 0);
    lv_obj_set_style_bg_opa(icon_wrap, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(icon_wrap, lv_color_hex(0x1A73E8), 0);
    lv_obj_set_style_shadow_width(icon_wrap, 6, 0);
    lv_obj_set_style_shadow_opa(icon_wrap, LV_OPA_20, 0);
    lv_obj_center(icon_wrap);

    lv_obj_t *icon_lbl = lv_label_create(icon_wrap);
    lv_label_set_text(icon_lbl, LV_SYMBOL_DIRECTORY);
    lv_obj_set_style_text_color(icon_lbl, lv_color_white(), 0);
    lv_obj_set_style_text_font(icon_lbl, lv_theme_get_font_small(icon_lbl), 0);
    lv_obj_center(icon_lbl);

    /* Right: title + subtitle (varied font sizes) */
    lv_obj_t *col = lv_obj_create(row);
    remove_all_styles(col, false);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 1, 0);

    /* Make the column take the remaining horizontal space */
    lv_obj_set_flex_grow(col, 1);

    lv_obj_t *title = lv_label_create(col);
    lv_label_set_text(title, "Remote File Client");
    lv_obj_set_width(title, LV_PCT(100)); // use full column width
    lv_obj_set_style_text_font(title, lv_theme_get_font_large(title), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x202124), 0);

    lv_obj_t *subtitle = lv_label_create(col);
    lv_label_set_text(subtitle, "Access your remote files securely.");
    lv_obj_set_width(title, LV_PCT(100)); // use full column width
    lv_obj_set_style_text_font(subtitle, lv_theme_get_font_small(subtitle), 0);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x5F6368), 0);
}

/* -------------------- events -------------------- */
static void on_login_closed(void *user_ctx)
{
    (void)user_ctx;
    g_rfiles.login_root = NULL;

    if (g_finalizing || g_rfiles.content == NULL)
        return;

    lv_obj_clean(g_rfiles.content);

    /* Inner card (only visible border) — compact to fit 800x600 comfortably */
    lv_obj_t *card = lv_obj_create(g_rfiles.content);
    lv_obj_set_size(card, LV_PCT(60), LV_SIZE_CONTENT);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_all(card, 12, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_100, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xDADCE0), 0);
    lv_obj_set_style_shadow_width(card, 8, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_20, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, "Welcome!\nUse the Login window to authenticate.");
    lv_obj_set_style_text_font(lbl, lv_theme_get_font_small(lbl), 0);
    lv_obj_center(lbl);
}

/* -------------------- public -------------------- */
void wnd_rfiles_show(void)
{
    memset(&g_rfiles, 0, sizeof(g_rfiles));
    g_finalizing = false;

    g_rfiles.screen = lv_obj_create(NULL);
    /* White background everywhere */
    lv_obj_set_style_bg_color(g_rfiles.screen, lv_color_hex(0xFFFFFF), 0);
    // lv_obj_set_style_bg_opa(g_rfiles.screen, LV_OPA_100, 0);
    lv_obj_set_size(g_rfiles.screen, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(g_rfiles.screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_scr_load(g_rfiles.screen);

    /* Root: column, no outer borders, no scrolling */
    lv_obj_t *root = g_rfiles.screen;
    remove_all_styles(root, true);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 8, 0); /* compact margins to fit 800x600 */
    lv_obj_set_style_pad_row(root, 8, 0);

    /* Banner (reduced height) */
    banner_build(root);

    /* Content area flexes to fill remaining space; no scroll */
    g_rfiles.content = lv_obj_create(root);
    remove_all_styles(g_rfiles.content, false);
    lv_obj_set_size(g_rfiles.content, LV_PCT(100), LV_PCT(100));
    /* Let center panel inside login consume remaining space deterministically */
    lv_obj_set_flex_grow(g_rfiles.content, 1);

    /* Show login window */
    g_rfiles.login_root = wnd_rfiles_login_create(g_rfiles.content, on_login_closed, NULL);
}

void wnd_rfiles_close_login_if_open(void)
{
    if (g_rfiles.login_root && wnd_rfiles_login_is_root(g_rfiles.login_root))
    {
        wnd_rfiles_login_close(g_rfiles.login_root);
        g_rfiles.login_root = NULL;
    }
}

void wnd_rfiles_close(void)
{
    g_finalizing = true;

    if (g_rfiles.login_root && wnd_rfiles_login_is_root(g_rfiles.login_root))
    {
        wnd_rfiles_login_close(g_rfiles.login_root);
        g_rfiles.login_root = NULL;
    }

    if (g_rfiles.screen)
    {
        lv_obj_t *active = lv_scr_act();
        if (active == g_rfiles.screen)
        {
            lv_obj_t *blank = lv_obj_create(NULL);
            lv_obj_set_style_bg_color(blank, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(blank, LV_OPA_100, 0);
            lv_scr_load(blank);
        }
        lv_obj_delete(g_rfiles.screen);
    }

    memset(&g_rfiles, 0, sizeof(g_rfiles));
    g_finalizing = false;
}

lv_obj_t *wnd_rfiles_get_root(void)
{
    return g_rfiles.content;
}
