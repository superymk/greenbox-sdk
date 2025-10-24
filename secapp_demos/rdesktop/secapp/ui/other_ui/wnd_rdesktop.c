// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>

#include <rdesktop_header.h>

static void *_rdesktop_fb = NULL;
static size_t _rdesktop_fb_size = 0;
static uint32_t wnd_width = 0;
static uint32_t wnd_height = 0;

lv_obj_t *ui_wnd_rdesktop = NULL;
static lv_obj_t *_rdesktop_canvas = NULL;

static SAP_STATUS _wnd_init(uint32_t in_wnd_width, uint32_t in_wnd_height)
{
    wnd_width = in_wnd_width;
    wnd_height = in_wnd_height;

    /* bytes per pixel for the display’s native format (RGB565/RGB888/… as configured) */
    size_t bpp = lv_color_format_get_size(LV_COLOR_FORMAT_NATIVE);
    _rdesktop_fb_size = (size_t)wnd_width * (size_t)wnd_height * bpp;
    if (_rdesktop_fb_size == 0)
        return EINVAL;

    _rdesktop_fb = malloc(_rdesktop_fb_size);
    if (!_rdesktop_fb)
        return ENOMEM;
    memset(_rdesktop_fb, 0, _rdesktop_fb_size);

    ui_wnd_rdesktop = lv_obj_create(NULL);

    _rdesktop_canvas = lv_canvas_create(ui_wnd_rdesktop);
    lv_obj_set_size(_rdesktop_canvas, wnd_width, wnd_height);

    /* v9: pass an lv_color_format_t instead of LV_IMG_CF_* */
    lv_canvas_set_buffer(_rdesktop_canvas,
                         _rdesktop_fb,
                         (int32_t)wnd_width,
                         (int32_t)wnd_height,
                         LV_COLOR_FORMAT_NATIVE);

    lv_obj_center(_rdesktop_canvas); /* or lv_obj_align(_rdesktop_canvas, LV_ALIGN_CENTER, 0, 0); */

    return SUCCESS;
}

SAP_STATUS wnd_rdesktop_create(uint32_t in_wnd_width, uint32_t in_wnd_height)
{
    SAP_STATUS status = SUCCESS;

    if (!ui_wnd_rdesktop)
    {
        status = _wnd_init(in_wnd_width, in_wnd_height);
        if (FAIL(status))
            return status;
    }

    // On success
    return SUCCESS;
}

void wnd_rdesktop_destroy(void)
{
    if (_rdesktop_fb)
    {
        free(_rdesktop_fb);
        _rdesktop_fb = NULL;
    }

    _rdesktop_fb_size = 0;
    wnd_width = 0;
    wnd_height = 0;
}

void *wnd_rdesktop_get_fb(void)
{
    return _rdesktop_fb;
}

void wnd_rdesktop_show(void)
{
    if (ui_wnd_rdesktop)
        lv_disp_load_scr(ui_wnd_rdesktop);
}

void wnd_rdesktop_refresh_all(void)
{
    if (_rdesktop_canvas)
    {
        lv_obj_invalidate(_rdesktop_canvas);
    }
}

void wnd_rdesktop_refresh_dirty(int x, int y, int w, int h)
{
    if (!_rdesktop_canvas)
        return;

    /* canvas screen coords */
    lv_area_t cc;
    lv_obj_get_coords(_rdesktop_canvas, &cc); // cc.x1,y1..cc.x2,y2 (screen space)

    /* build screen-space dirty rect */
    lv_area_t dirty;
    dirty.x1 = cc.x1 + x;
    dirty.y1 = cc.y1 + y;
    dirty.x2 = dirty.x1 + w - 1;
    dirty.y2 = dirty.y1 + h - 1;

    /* clip to canvas bounds (screen space) */
    if (dirty.x1 < cc.x1)
        dirty.x1 = cc.x1;
    if (dirty.y1 < cc.y1)
        dirty.y1 = cc.y1;
    if (dirty.x2 > cc.x2)
        dirty.x2 = cc.x2;
    if (dirty.y2 > cc.y2)
        dirty.y2 = cc.y2;

    if (dirty.x1 > dirty.x2 || dirty.y1 > dirty.y2)
        return;

    lv_obj_invalidate_area(_rdesktop_canvas, &dirty);
}
