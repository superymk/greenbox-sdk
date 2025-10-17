// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/************************************************************************************
 * Remote File Client — List Window (LVGL v9.4)
 * - Left: Local files via lv_file_explorer (Quick Access hidden)
 *          • Compact rows, blue selected-row highlight, white text on selection (optional)
 *          • Selection stores absolute path of selected FILE (not directory)
 *          • Clears highlight when directory changes
 * - Right: Remote (Google Drive) via lv_table
 *          • Row 0: header   | Name | Date modified | Size
 *          • Row 1: ".." (go up)
 *          • Row 2..N: entries
 *          • Compact rows, blue selected-row highlight, zebra striping, centered header
 * - Middle: Two-column buttons grid between panes
 *          • Row 1: [Upload] [Download]
 *          • Row 2: [   -   ] [Delete ]
 * - Bottom: progress text + bar
 ************************************************************************************/
#include "wnd_rfiles_list.h"

#include <rfiles_header.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/stat.h>

/* ---------------------------------- Config --------------------------------- */

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* White text on selected row (for contrast) */
#ifndef USE_WHITE_TEXT_ON_SELECTED
#define USE_WHITE_TEXT_ON_SELECTED 1
#endif

/* Double-click threshold (ms) for the right remote table */
#define DC_THRESHOLD_MS 300

#ifndef SELECTED_PATH_MAX
#define SELECTED_PATH_MAX 512
#endif

/* LVGL exposes class objects; needed to detect table type during traversal */
extern const lv_obj_class_t lv_table_class;

/* --------------------------------- GDrive ---------------------------------- */
/* Get children of parent_id ("root" allowed). On success returns 0 and
 * fills *out (malloc'ed array) and *out_n. Caller frees via gdrive_entries_free().
 */
extern int gdrive_list_children(const char *parent_id, gdrive_entry_t **out, size_t *out_n);

/* Free array returned from gdrive_list_children */
extern void gdrive_entries_free(gdrive_entry_t *arr, size_t n);

/* Download a file/folder (recursive for folders) to a local directory path */
extern int gdrive_download_to(const char *id, const char *local_dir);

/* Upload one local file into target folder id */
extern int gdrive_upload_one(const char *local_file, const char *remote_folder_id);

/* Delete one file/folder by id (non-recursive or server-recursive per Drive API policy).
   Return 0 on success. */
extern int gdrive_delete_one(const char *id);

/* --------------------------------- Types ----------------------------------- */
typedef struct
{
    lv_obj_t *root;

    /* Left pane */
    lv_obj_t *left_explorer;                /* lv_file_explorer */
    lv_obj_t *fe_table;                     /* internal table used by FE */
    uint32_t fe_sel_row;                    /* blue highlight row in FE table (files only) */
    char local_sel_path[SELECTED_PATH_MAX]; /* absolute path (real file system) to selected FILE */
    bool has_local_sel;

    /* Mid buttons (two-column grid) */
    lv_obj_t *mid_grid;
    lv_obj_t *btn_upload;   /* -> */
    lv_obj_t *btn_download; /* <- */
    lv_obj_t *btn_delete;   /* trash */

    /* Right pane */
    lv_obj_t *right_table; /* lv_table */
    uint32_t sel_row;      /* table row (0=hdr, 1=.., >=2 data) */

    /* Status area */
    lv_obj_t *status_label;
    lv_obj_t *status_bar;

    /* Remote browsing state */
    char cur_remote_id[128]; /* current folder id */
    gdrive_entry_t *cache;   /* current listing (data only) */
    size_t cache_n;

    /* “..” navigation stack */
    char parent_stack[16][128];
    uint8_t parent_top; /* size of the stack */

    /* Styles */
    lv_style_t hdr_style;  /* (kept for compatibility; header is also tinted via draw hook) */
    lv_style_t item_style; /* default item bg/text */
} wnd_list_t;

/// @brief LVGL filepath of the home folder of the local computer.
static char _lv_fs_local_home_dir[RFILES_DIR_FILEPATH_MAXLEN] = {0};

/* --------------------------- Forward decls -------------------------------- */
static void set_status(wnd_list_t *w, const char *fmt, ...);
static void refresh_remote_table(wnd_list_t *w);
static void refresh_local_fe(wnd_list_t *w);
static void push_parent(wnd_list_t *w, const char *id);
static const char *pop_parent(wnd_list_t *w);

/* FE helpers */
static lv_obj_t *find_descendant_table(lv_obj_t *root);
static bool path_join(char *dst, size_t dstsz, const char *base, const char *name);
static bool _lv_is_directory(const char *lv_abs_path);

/* Draw hooks / styles */
static void init_small_row_style(void);
static void fe_table_draw_event_cb(lv_event_t *e);
static void fe_table_click_or_change_cb(lv_event_t *e);
static void fe_value_changed_cb(lv_event_t *e);

static void remote_table_draw_event_cb(lv_event_t *e);
static void on_remote_table_clicked(lv_event_t *e);

/* Actions */
static void on_upload_clicked(lv_event_t *e);
static void on_download_clicked(lv_event_t *e);
static void on_delete_clicked(lv_event_t *e);

/* Layout */
static void build_layout(wnd_list_t *w, lv_obj_t *parent);

/* ------------------------------ Small-row style ---------------------------- */
static lv_style_t st_rows_small;
static bool st_rows_small_inited = false;

static void init_small_row_style(void)
{
    if (st_rows_small_inited)
        return;
    st_rows_small_inited = true;

    lv_style_init(&st_rows_small);
    lv_style_set_pad_top(&st_rows_small, 1);
    lv_style_set_pad_bottom(&st_rows_small, 1);
    lv_style_set_pad_left(&st_rows_small, 4);
    lv_style_set_pad_right(&st_rows_small, 4);
    lv_style_set_text_line_space(&st_rows_small, 0);
    lv_style_set_border_width(&st_rows_small, 0);

    extern const lv_font_t lv_font_montserrat_16;
    lv_style_set_text_font(&st_rows_small, &lv_font_montserrat_16);
}

/* -------------------------------- Helpers ---------------------------------- */

static bool path_join(char *dst, size_t dstsz, const char *base, const char *name)
{
    if (!dst || !base || !name)
        return false;

    size_t lb = lv_strlen(base);
    size_t ln = lv_strlen(name);

    /* reserve 2 extra (slash + NUL) */
    if (lb + 1 + ln + 1 > dstsz)
        return false;

    lv_snprintf(dst, dstsz, "%s", base);

    if (lb > 0 && base[lb - 1] != '/')
        lv_snprintf(dst + lb, dstsz - lb, "/%s", name);
    else
        lv_snprintf(dst + lb, dstsz - lb, "%s", name);

    return true;
}

static bool _lv_is_directory(const char *lv_abs_path)
{
    if (!lv_abs_path)
        return false;
    lv_fs_dir_t d;
    lv_fs_res_t r = lv_fs_dir_open(&d, lv_abs_path);
    if (r == LV_FS_RES_OK)
    {
        lv_fs_dir_close(&d);
        return true;
    }
    return false;
}

/* Depth-first search for the first descendant table */
static lv_obj_t *find_descendant_table(lv_obj_t *root)
{
    if (!root)
        return NULL;

    if (lv_obj_check_type(root, &lv_table_class))
        return root;

    uint32_t cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(root, i);
        lv_obj_t *t = find_descendant_table(child);
        if (t)
            return t;
    }
    return NULL;
}

/* --------------------------- Remote table styles --------------------------- */

static void table_apply_header_style(wnd_list_t *w)
{
    /* Header style (kept for compatibility with previous code; header will also
       be centered and gently tinted via draw hook) */
    lv_style_init(&w->hdr_style);
    lv_style_set_bg_color(&w->hdr_style, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_color(&w->hdr_style, lv_color_white());
    lv_style_set_pad_all(&w->hdr_style, 6);
    lv_style_set_border_width(&w->hdr_style, 0);
    lv_style_set_radius(&w->hdr_style, 4);

    lv_table_set_row_cnt(w->right_table, 1);
    lv_table_set_col_cnt(w->right_table, 3);
    lv_table_set_col_width(w->right_table, 0, 240);
    lv_table_set_col_width(w->right_table, 1, 160);
    lv_table_set_col_width(w->right_table, 2, 100);

    lv_table_set_cell_value(w->right_table, 0, 0, "Name");
    lv_table_set_cell_value(w->right_table, 0, 1, "Date modified");
    lv_table_set_cell_value(w->right_table, 0, 2, "Size");

    /* Mark header cells as CUSTOM_1 -> maps to LV_STATE_USER_1 for styling */
    lv_table_set_cell_ctrl(w->right_table, 0, 0, LV_TABLE_CELL_CTRL_CUSTOM_1);
    lv_table_set_cell_ctrl(w->right_table, 0, 1, LV_TABLE_CELL_CTRL_CUSTOM_1);
    lv_table_set_cell_ctrl(w->right_table, 0, 2, LV_TABLE_CELL_CTRL_CUSTOM_1);

    /* Apply header style ONLY to header cells */
    lv_obj_add_style(w->right_table, &w->hdr_style, LV_PART_ITEMS | LV_STATE_USER_1);

    /* Default item style (white rows) */
    lv_style_init(&w->item_style);
    lv_style_set_bg_color(&w->item_style, lv_color_white());
    lv_style_set_text_color(&w->item_style, lv_color_black());
    lv_style_set_border_width(&w->item_style, 0);
    lv_obj_add_style(w->right_table, &w->item_style, LV_PART_ITEMS | LV_STATE_DEFAULT);
}

/* ------------------------ Remote table rendering hook ---------------------- */

static void remote_table_draw_event_cb(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
    if (!base_dsc)
        return;

    if (base_dsc->part == LV_PART_ITEMS)
    {
        uint32_t row = base_dsc->id1;
        uint32_t col = base_dsc->id2;

        /* ---- Selected row override (paint blue) ---- */
        if (row == w->sel_row && row != 0)
        {
            lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(draw_task);
            if (fill)
            {
                fill->opa = LV_OPA_COVER;
                fill->color = lv_palette_main(LV_PALETTE_BLUE);
            }
#if USE_WHITE_TEXT_ON_SELECTED
            lv_draw_label_dsc_t *lab = lv_draw_task_get_label_dsc(draw_task);
            if (lab)
            {
                if (col == 0)
                    lab->align = LV_TEXT_ALIGN_LEFT;
                lab->color = lv_color_white();
            }
#endif
            return; /* don't apply header/zebra to selected row */
        }

        /* ---- Header (row 0): center + gentle blue tint ---- */
        if (row == 0)
        {
            lv_draw_label_dsc_t *label = lv_draw_task_get_label_dsc(draw_task);
            if (label)
                label->align = LV_TEXT_ALIGN_CENTER;

            lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(draw_task);
            if (fill)
            {
                fill->color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE),
                                           fill->color, LV_OPA_20);
                fill->opa = LV_OPA_COVER;
            }
        }
        /* ---- First column left align ---- */
        else if (col == 0)
        {
            lv_draw_label_dsc_t *label = lv_draw_task_get_label_dsc(draw_task);
            if (label)
                label->align = LV_TEXT_ALIGN_LEFT;
        }

        /* ---- Zebra striping (skip header) ---- */
        if (row != 0 && (row % 2) == 0)
        {
            lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(draw_task);
            if (fill)
            {
                fill->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY),
                                           fill->color, LV_OPA_10);
                fill->opa = LV_OPA_COVER;
            }
        }
    }
}

/* Single-click on remote table: update selection and repaint.
   Double-click: navigate (".." or open folder). */
static void on_remote_table_clicked(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    lv_obj_t *tbl = lv_event_get_target(e);

    static lv_obj_t *s_last_target = NULL;
    static uint32_t s_last_click_ms = 0;
    static uint32_t s_last_row = UINT32_MAX;

    /* reset per different widget */
    if (tbl != s_last_target)
    {
        s_last_target = tbl;
        s_last_click_ms = 0;
        s_last_row = UINT32_MAX;
    }

    uint32_t row = LV_TABLE_CELL_NONE, col = LV_TABLE_CELL_NONE;
    lv_table_get_selected_cell(w->right_table, &row, &col);

    /* Cache selection for single-click */
    w->sel_row = row;

    uint32_t now = lv_tick_get();
    bool same_row = (row == s_last_row);
    bool is_double = same_row && (now - s_last_click_ms <= DC_THRESHOLD_MS);

    s_last_row = row;
    s_last_click_ms = now;

    if (!is_double)
    {
        /* just selection; repaint to show blue */
        lv_obj_invalidate(w->right_table);
        return;
    }

    /* double click => navigate */
    if (row == 1)
    { /* “..” */
        const char *pid = pop_parent(w);
        lv_strlcpy(w->cur_remote_id, pid, sizeof(w->cur_remote_id));
        refresh_remote_table(w);
        return;
    }

    if (row >= 2 && (row - 2) < w->cache_n)
    {
        const gdrive_entry_t *einfo = &w->cache[row - 2];
        if (einfo->is_folder && einfo->id)
        {
            push_parent(w, w->cur_remote_id);
            lv_strlcpy(w->cur_remote_id, einfo->id, sizeof(w->cur_remote_id));
            refresh_remote_table(w);
        }
    }
}

/* -------------------------- File-explorer hooks --------------------------- */

/* Paint blue selected FILE row in FE's internal table; dirs are never highlighted */
static void fe_table_draw_event_cb(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
    if (!base_dsc)
        return;

    if (base_dsc->part == LV_PART_ITEMS)
    {
        uint32_t row = base_dsc->id1;
        uint32_t col = base_dsc->id2;

        /* Selected file row: paint blue */
        if (w->fe_sel_row != LV_TABLE_CELL_NONE && row == w->fe_sel_row)
        {
            lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(draw_task);
            if (fill)
            {
                fill->opa = LV_OPA_COVER;
                fill->color = lv_palette_main(LV_PALETTE_BLUE);
            }
#if USE_WHITE_TEXT_ON_SELECTED
            lv_draw_label_dsc_t *lab = lv_draw_task_get_label_dsc(draw_task);
            if (lab)
            {
                if (col == 0)
                    lab->align = LV_TEXT_ALIGN_LEFT;
                lab->color = lv_color_white();
            }
#endif
            return; /* don't apply header/zebra to the selected row */
        }

        /* Header (row 0) style nudge */
        if (row == 0)
        {
            lv_draw_label_dsc_t *label = lv_draw_task_get_label_dsc(draw_task);
            if (label)
                label->align = LV_TEXT_ALIGN_CENTER;

            lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(draw_task);
            if (fill)
            {
                fill->color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE),
                                           fill->color, LV_OPA_20);
                fill->opa = LV_OPA_COVER;
            }
        }
        else if (col == 0)
        {
            /* First column left align (names) */
            lv_draw_label_dsc_t *label = lv_draw_task_get_label_dsc(draw_task);
            if (label)
                label->align = LV_TEXT_ALIGN_LEFT;
        }

        /* Zebra striping (skip header) */
        if (row != 0 && (row % 2) == 0)
        {
            lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(draw_task);
            if (fill)
            {
                fill->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY),
                                           fill->color, LV_OPA_10);
                fill->opa = LV_OPA_COVER;
            }
        }
    }
}

#define LV_FS_MAX_PATH_LEN 256

/// @brief Convert lvgl absolute file path <lv_abs_path> to the file path in the underlying file system.
static void _lvgl_fs_abs_path_to_fs_abs_path(char *dst_fs_abs_path, const char *lv_abs_path, uint32_t dst_fs_abs_path_size)
{
    if (!dst_fs_abs_path || !lv_abs_path)
        return;
    if (dst_fs_abs_path_size <= 1) // The size of <dst_fs_abs_path> must >= 2 because it needs one byte for '\0'.
        return;

    if (strnlen(lv_abs_path, LV_FS_MAX_PATH_LEN) <= 2)
    {
        // Return "/" if the lvgl absolute file path is empty or contains the driver letter only.
        dst_fs_abs_path[0] = '/';
        dst_fs_abs_path[1] = '\0';
        return;
    }

#ifdef LV_FS_POSIX_PATH
    lv_snprintf(dst_fs_abs_path, dst_fs_abs_path_size, LV_FS_POSIX_PATH "%s", &lv_abs_path[2]);
#else
    /* Fallback: strip "X:" and use '/' root */
    lv_snprintf(dst_fs_abs_path, dst_fs_abs_path_size, "%s", &lv_abs_path[2]);
#endif
}

static void _fs_abs_path_to_lvgl_abs_path(char *dst_lv_abs_path, const char *fs_abs_path, uint32_t dst_lv_abs_path_size)
{
    if (!dst_lv_abs_path || !fs_abs_path)
        return;
    if (dst_lv_abs_path_size <= 1) // The size of <dst_fs_abs_path> must >= 2 because it needs one byte for '\0'.
        return;
    
    snprintf(dst_lv_abs_path, dst_lv_abs_path_size, "%s%s", "A:", fs_abs_path);
}

/* Click or VALUE_CHANGED on FE internal table: compute absolute FILE path, set highlight.
   If directory, clear highlight and let FE handle navigation as usual. */
static void fe_table_click_or_change_cb(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w || !w->left_explorer)
        return;

    uint32_t r = LV_TABLE_CELL_NONE, c = LV_TABLE_CELL_NONE;
    lv_table_get_selected_cell(w->fe_table, &r, &c);

    if (r == LV_TABLE_CELL_NONE || r == 0)
    {
        /* header or nothing: clear selection */
        w->fe_sel_row = LV_TABLE_CELL_NONE;
        w->local_sel_path[0] = '\0';
        w->has_local_sel = false;
        lv_obj_invalidate(w->fe_table);
        return;
    }

    const char *cur_path = lv_file_explorer_get_current_path(w->left_explorer);
    const char *name = lv_file_explorer_get_selected_file_name(w->left_explorer);
    if (!cur_path || !name || !*name)
    {
        w->fe_sel_row = LV_TABLE_CELL_NONE;
        w->local_sel_path[0] = '\0';
        w->has_local_sel = false;
        lv_obj_invalidate(w->fe_table);
        return;
    }

    char lv_abs_path[SELECTED_PATH_MAX] = {0};
    if (!path_join(lv_abs_path, sizeof(lv_abs_path), cur_path, name))
    {
        /* too long */
        w->fe_sel_row = LV_TABLE_CELL_NONE;
        w->local_sel_path[0] = '\0';
        w->has_local_sel = false;
        lv_obj_invalidate(w->fe_table);
        return;
    }

    /* If it's a directory, DO NOT highlight as a file; FE will open/navigate on its own */
    if (_lv_is_directory(lv_abs_path))
    {
        w->fe_sel_row = LV_TABLE_CELL_NONE;
        w->local_sel_path[0] = '\0';
        w->has_local_sel = false;
        lv_obj_invalidate(w->fe_table);
        return;
    }

    /* It's a file: store global absolute path + paint row */
    w->fe_sel_row = r;
    _lvgl_fs_abs_path_to_fs_abs_path(w->local_sel_path, lv_abs_path, sizeof(w->local_sel_path));
    w->has_local_sel = true;

    lv_obj_invalidate(w->fe_table);
}

/* When the FE path/dir changes, clear file selection/highlight */
static void fe_value_changed_cb(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    w->fe_sel_row = LV_TABLE_CELL_NONE;
    w->local_sel_path[0] = '\0';
    w->has_local_sel = false;
    if (w->fe_table)
        lv_obj_invalidate(w->fe_table);
}

/* --------------------------- Remote table logic ---------------------------- */
static lv_obj_t *make_icon_button(lv_obj_t *parent, const char *symbol)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 56, 56);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, symbol);
    lv_obj_center(lbl);
    return btn;
}

static void free_cache(wnd_list_t *w)
{
    if (!w->cache)
        return;
    gdrive_entries_free(w->cache, w->cache_n);
    w->cache = NULL;
    w->cache_n = 0;
}

static void human_size_fmt(uint64_t sz, char *out, size_t outsz)
{
    static const char *u[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    unsigned ui = 0;
    uint64_t unit = 1ULL; /* 1024^ui */

    /* Find the largest unit where value < 1024 or we hit PB */
    while (ui < 5 && sz >= (unit << 10))
    {
        unit <<= 10; /* *= 1024 */
        ui++;
    }

    if (ui == 0)
    { /* bytes: plain integer */
        lv_snprintf(out, outsz, "%" PRIu64 " %s", sz, u[ui]);
        return;
    }

    uint64_t whole = sz / unit;
    if (whole >= 10)
    {
        /* >= 10.x -> no decimal */
        lv_snprintf(out, outsz, "%" PRIu64 " %s", whole, u[ui]);
    }
    else
    {
        /* < 10.x -> one decimal digit, rounded */
        uint64_t rem = sz - whole * unit;
        uint32_t tenth = (uint32_t)((rem * 10ULL + unit / 2ULL) / unit); /* 0..10 */

        if (tenth == 10)
        { /* handle 9.95 -> 10.0 carry */
            whole += 1;
            tenth = 0;
        }
        lv_snprintf(out, outsz, "%" PRIu64 ".%u %s", whole, tenth, u[ui]);
    }
}

static void _msgbox_show_info(const char *info_type, const char *str_err)
{
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mbox, info_type);
    lv_msgbox_add_text(mbox, str_err);
    lv_msgbox_add_close_button(mbox);
    lv_obj_center(mbox);
}

/* --------------------------- File-explorer refresh ------------------------- */
static void refresh_local_fe(wnd_list_t *w)
{
    if (!w || !w->left_explorer)
        return;

    /* Re-open current dir to force FE to refresh its listing */
    const char *cur = lv_file_explorer_get_current_path(w->left_explorer);
    if (cur && *cur)
    {
        lv_file_explorer_open_dir(w->left_explorer, cur);
    }

    /* Clear any cached file selection/highlight */
    w->fe_sel_row = LV_TABLE_CELL_NONE;
    w->local_sel_path[0] = '\0';
    w->has_local_sel = false;

    if (w->fe_table)
        lv_obj_invalidate(w->fe_table);
}

/* Fill right table: row0 header, row1 "..", rows>=2 data */
static void refresh_remote_table(wnd_list_t *w)
{
    free_cache(w);

    if (gdrive_list_children(w->cur_remote_id, &w->cache, &w->cache_n) != 0)
    {
        set_status(w, "Remote list failed.");
        /* keep headers intact */
        lv_table_set_row_cnt(w->right_table, 2);
        lv_table_set_cell_value(w->right_table, 1, 0, "..");
        lv_table_set_cell_value(w->right_table, 1, 1, "");
        lv_table_set_cell_value(w->right_table, 1, 2, "");
        w->sel_row = 1;
        lv_obj_invalidate(w->right_table);
        return;
    }

    /* header + (“..”) + items */
    lv_table_set_row_cnt(w->right_table, (uint16_t)(w->cache_n + 2));
    lv_table_set_cell_value(w->right_table, 1, 0, "..");
    lv_table_set_cell_value(w->right_table, 1, 1, "");
    lv_table_set_cell_value(w->right_table, 1, 2, "");

    for (size_t i = 0; i < w->cache_n; i++)
    {
        uint16_t r = (uint16_t)(i + 2);
        const gdrive_entry_t *e = &w->cache[i];

        /* Name (add a folder glyph) */
        if (e->is_folder)
        {
            lv_table_set_cell_value_fmt(w->right_table, r, 0, LV_SYMBOL_DIRECTORY " %s", e->name ? e->name : "");
        }
        else
        {
            lv_table_set_cell_value(w->right_table, r, 0, e->name ? e->name : "");
        }

        /* Modified */
        lv_table_set_cell_value(w->right_table, r, 1, e->modified_time ? e->modified_time : "");

        /* Size */
        if (e->is_folder)
        {
            lv_table_set_cell_value(w->right_table, r, 2, "");
        }
        else
        {
            char s[32];
            human_size_fmt(e->size, s, sizeof(s));
            lv_table_set_cell_value(w->right_table, r, 2, s);
        }
    }
    w->sel_row = 1; /* default select (“..”) */
    lv_obj_invalidate(w->right_table);
}

/* ---------------------------- Status helper -------------------------------- */
static void set_status(wnd_list_t *w, const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    lv_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    lv_label_set_text(w->status_label, buf);
}

/* -------------------------- Parent stack helpers --------------------------- */
static void push_parent(wnd_list_t *w, const char *id)
{
    if (w->parent_top >= (uint8_t)(sizeof(w->parent_stack) / sizeof(w->parent_stack[0])))
        return;
    lv_strlcpy(w->parent_stack[w->parent_top], id, sizeof(w->parent_stack[0]));
    w->parent_top++;
}
static const char *pop_parent(wnd_list_t *w)
{
    if (w->parent_top == 0)
        return "root";
    w->parent_top--;
    return w->parent_stack[w->parent_top];
}

/* -------------------------------- Actions ---------------------------------- */

static void on_upload_clicked(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    if (!w->has_local_sel || !w->local_sel_path[0])
    {
        set_status(w, "Select a local file.");
        return;
    }

    struct stat st;
    if (stat(w->local_sel_path, &st) != 0)
    {
        set_status(w, "Cannot stat: %s", w->local_sel_path);
        return;
    }
    if (!S_ISREG(st.st_mode))
    {
        set_status(w, "Choose a regular file to upload.");
        return;
    }

    set_status(w, "Uploading %s ...", w->local_sel_path);
    lv_bar_set_value(w->status_bar, 15, LV_ANIM_OFF);

    if (gdrive_upload_one(w->local_sel_path, w->cur_remote_id) == 0)
    {
        lv_bar_set_value(w->status_bar, 100, LV_ANIM_OFF);
        set_status(w, "Upload OK.");
        refresh_remote_table(w);
    }
    else
    {
        lv_bar_set_value(w->status_bar, 0, LV_ANIM_OFF);
        set_status(w, "Upload failed.");
        _msgbox_show_info("Upload Failed", "File size >= 4MB or connection error!");
    }
}

static void on_download_clicked(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    /* Need a target local directory path */
    const char *cur = lv_file_explorer_get_current_path(w->left_explorer);
    if (!cur || !*cur)
    {
        set_status(w, "Pick a local directory.");
        return;
    }
    char local_dir[SELECTED_PATH_MAX];
    _lvgl_fs_abs_path_to_fs_abs_path(local_dir, cur, SELECTED_PATH_MAX);

    /* Require a selection on the right pane (not header nor “..”) */
    if (w->sel_row < 2 || (w->sel_row - 2) >= w->cache_n)
    {
        set_status(w, "Select a remote item.");
        return;
    }

    const gdrive_entry_t *einfo = &w->cache[w->sel_row - 2];
    set_status(w, "Downloading %s ...", einfo->name ? einfo->name : "(item)");
    lv_bar_set_value(w->status_bar, 10, LV_ANIM_OFF);

    if (gdrive_download_to(einfo->id, local_dir) == 0)
    {
        lv_bar_set_value(w->status_bar, 100, LV_ANIM_OFF);
        set_status(w, "Saved to %s", local_dir);

        // Refresh file explorer of the local files.
        refresh_local_fe(w);
    }
    else
    {
        lv_bar_set_value(w->status_bar, 0, LV_ANIM_OFF);
        set_status(w, "Download failed.");
        _msgbox_show_info("Download Failed", "File size >= 4MB or connection error!");
    }
}

static void on_delete_clicked(lv_event_t *e)
{
    wnd_list_t *w = (wnd_list_t *)lv_event_get_user_data(e);
    if (!w)
        return;

    /* Require a selection on the right pane (not header nor “..”) */
    if (w->sel_row < 2 || (w->sel_row - 2) >= w->cache_n)
    {
        set_status(w, "Select a remote item to delete.");
        return;
    }

    const gdrive_entry_t *einfo = &w->cache[w->sel_row - 2];
    if (!einfo->id || !*einfo->id)
    {
        set_status(w, "Delete failed: invalid item.");
        return;
    }

    set_status(w, "Deleting %s ...", einfo->name ? einfo->name : "(item)");
    lv_bar_set_value(w->status_bar, 5, LV_ANIM_OFF);

    int rc = gdrive_delete_one(einfo->id);
    if (rc == 0)
    {
        lv_bar_set_value(w->status_bar, 100, LV_ANIM_OFF);
        set_status(w, "Deleted.");
        refresh_remote_table(w);
    }
    else
    {
        lv_bar_set_value(w->status_bar, 0, LV_ANIM_OFF);
        set_status(w, "Delete failed.");
    }
}

/* ----------------------------- Layout builder ------------------------------ */

static void build_layout(wnd_list_t *w, lv_obj_t *parent)
{
    init_small_row_style();

    w->root = lv_obj_create(parent);
    lv_obj_set_size(w->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(w->root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(w->root, 8, 0);
    lv_obj_set_style_pad_row(w->root, 8, 0);

    /* Top content row: [FileExplorer] [buttons grid] [Table] */
    lv_obj_t *row = lv_obj_create(w->root);
    lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(row, 1);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 12, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    /* Center children vertically in the row (cross-axis) */
    lv_obj_set_flex_align(row,
                          LV_FLEX_ALIGN_START,   /* main axis: left-to-right */
                          LV_FLEX_ALIGN_CENTER,  /* cross axis: center vertically */
                          LV_FLEX_ALIGN_CENTER); /* track cross: center */

    /* Left file explorer */
    w->left_explorer = lv_file_explorer_create(row);
    lv_obj_set_flex_grow(w->left_explorer, 1); /* ← grow to take half of remaining width */
    lv_obj_set_height(w->left_explorer, LV_PCT(100));
    lv_file_explorer_set_sort(w->left_explorer, LV_EXPLORER_SORT_KIND);

#if LVGL_VERSION_MAJOR >= 9
#ifdef lv_file_explorer_set_show_quick_access
    lv_file_explorer_set_show_quick_access(w->left_explorer, false);
#elif defined(lv_file_explorer_set_quick_access_visible)
    lv_file_explorer_set_quick_access_visible(w->left_explorer, false);
#endif
#endif

    /* Open a real directory so entries appear */
    lv_file_explorer_open_dir(w->left_explorer, _lv_fs_local_home_dir);

    /* Clear file highlight when FE signals a path/value change */
    lv_obj_add_event_cb(w->left_explorer, fe_value_changed_cb, LV_EVENT_VALUE_CHANGED, w);

    /* Find the internal table and attach hooks */
    w->fe_table = find_descendant_table(w->left_explorer);
    w->fe_sel_row = LV_TABLE_CELL_NONE;
    w->local_sel_path[0] = '\0';
    w->has_local_sel = false;

    if (w->fe_table)
    {
        lv_obj_add_event_cb(w->fe_table, fe_table_draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, w);
        lv_obj_add_flag(w->fe_table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

        lv_obj_add_event_cb(w->fe_table, fe_table_click_or_change_cb, LV_EVENT_PRESSED, w);
        lv_obj_add_event_cb(w->fe_table, fe_table_click_or_change_cb, LV_EVENT_VALUE_CHANGED, w);

        /* Compact rows for denser list */
        lv_obj_add_style(w->fe_table, &st_rows_small, LV_PART_ITEMS);

        /* FE's internal table should already be clickable/focusable; ensure anyway */
        lv_obj_add_flag(w->fe_table, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(w->fe_table, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    }
    else
    {
        /* If this triggers, FE internals changed; adjust find_descendant_table() */
        SAP_WARN("Internal FE table not found; no row highlight\n");
    }

    /* Middle buttons GRID: two columns
       Row 1: [Upload] [Download]
       Row 2: [  -  ]  [ Delete ] */

    w->mid_grid = lv_obj_create(row);
    lv_obj_set_style_pad_all(w->mid_grid, 0, 0);
    lv_obj_set_style_border_width(w->mid_grid, 0, 0);
    lv_obj_set_size(w->mid_grid, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    /* Ensure the center column does NOT grow */
    lv_obj_set_flex_grow(w->mid_grid, 0);
    /* (optional) fix its width so “equal halves” are visually equal */
    lv_obj_set_style_min_width(w->mid_grid, 144, 0); /* ~2×64 + gaps */

    /* Set a small grid: 2 columns of 64px, 2 rows of 64px, with gap */
    static int32_t cols[] = {64, 64, LV_GRID_TEMPLATE_LAST};
    static int32_t rows[] = {64, 64, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(w->mid_grid, cols, rows);
    lv_obj_set_style_pad_column(w->mid_grid, 8, 0);
    lv_obj_set_style_pad_row(w->mid_grid, 12, 0);

    /* Upload button (col 0, row 0) */
    w->btn_upload = make_icon_button(w->mid_grid, LV_SYMBOL_RIGHT);
    lv_obj_set_grid_cell(w->btn_upload,
                         LV_GRID_ALIGN_CENTER, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    /* Download button (col 1, row 0) */
    w->btn_download = make_icon_button(w->mid_grid, LV_SYMBOL_LEFT);
    lv_obj_set_grid_cell(w->btn_download,
                         LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    /* Delete button (col 1, row 1) */
    w->btn_delete = make_icon_button(w->mid_grid, LV_SYMBOL_TRASH);
    lv_obj_set_grid_cell(w->btn_delete,
                         LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    /* Right remote table */
    w->right_table = lv_table_create(row);
    lv_obj_set_height(w->right_table, LV_PCT(100));
    lv_obj_set_flex_grow(w->right_table, 1); /* grow to take the other half */
    lv_obj_add_flag(w->right_table, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    /* Header & default item styles as before (plus draw hook tweaks) */
    table_apply_header_style(w);

    /* Add the “..” row now (refresh will repopulate) */
    lv_table_set_row_cnt(w->right_table, 2);
    lv_table_set_cell_value(w->right_table, 1, 0, "..");
    lv_table_set_cell_value(w->right_table, 1, 1, "");
    lv_table_set_cell_value(w->right_table, 1, 2, "");

    /* Compact rows for the remote table */
    lv_obj_add_style(w->right_table, &st_rows_small, LV_PART_ITEMS);

    /* Draw hook for blue highlight + zebra + header tint */
    lv_obj_add_event_cb(w->right_table, remote_table_draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, w);
    lv_obj_add_flag(w->right_table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    /* Single-click selection + double-click navigation on right table */
    lv_obj_add_event_cb(w->right_table, on_remote_table_clicked, LV_EVENT_PRESSED, w);

    /* Bottom status bar */
    lv_obj_t *bot = lv_obj_create(w->root);
    lv_obj_set_size(bot, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(bot, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(bot, 0, 0);
    lv_obj_set_style_border_width(bot, 0, 0);

    w->status_label = lv_label_create(bot);
    lv_label_set_text(w->status_label, "Ready.");

    w->status_bar = lv_bar_create(bot);
    lv_obj_set_width(w->status_bar, LV_PCT(100));
    lv_bar_set_range(w->status_bar, 0, 100);
    lv_bar_set_value(w->status_bar, 0, LV_ANIM_OFF);

    /* Wire up buttons */
    lv_obj_add_event_cb(w->btn_upload, on_upload_clicked, LV_EVENT_CLICKED, w);
    lv_obj_add_event_cb(w->btn_download, on_download_clicked, LV_EVENT_CLICKED, w);
    lv_obj_add_event_cb(w->btn_delete, on_delete_clicked, LV_EVENT_CLICKED, w);
}

/* ----------------------------- Public API ---------------------------------- */

static lv_obj_t *wnd_rfiles_list_create(lv_obj_t *parent)
{
    wnd_list_t *w = (wnd_list_t *)lv_malloc(sizeof(*w));
    LV_ASSERT(w);
    memset(w, 0, sizeof(*w));
    lv_strlcpy(w->cur_remote_id, "root", sizeof(w->cur_remote_id));

    build_layout(w, parent);

    /* Initial fill of remote table */
    refresh_remote_table(w);

    return w->root;
}

SAP_STATUS wnd_rfiles_list_init_cfgs(SAP_CONFIG* sap_cfg)
{
    if (!sap_cfg)
        return EINVAL;

    // Initialize LVGL file system paths.
    _fs_abs_path_to_lvgl_abs_path(_lv_fs_local_home_dir, sap_cfg->fs_local_info.home_dir, RFILES_DIR_FILEPATH_MAXLEN);

    // On success
    return SUCCESS;
}

void wnd_rfiles_list_show(lv_obj_t *parent)
{
    lv_obj_clean(parent);
    (void)wnd_rfiles_list_create(parent);
}
