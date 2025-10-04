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

    _rdesktop_fb_size = wnd_width * wnd_height * lv_img_cf_get_px_size(LV_IMG_CF_TRUE_COLOR);
    if (!_rdesktop_fb_size)
        return EINVAL;

    _rdesktop_fb = malloc(_rdesktop_fb_size);
    if (!_rdesktop_fb)
        return ENOMEM;
    memset(_rdesktop_fb, 0, _rdesktop_fb_size);

    ui_wnd_rdesktop = lv_obj_create(NULL);
    // ui_wnd_rdesktop = lv_win_create(lv_scr_act(), NULL);

    // Create a canvas widget inside the window. The canvas will display the remote desktop image.
    _rdesktop_canvas = lv_canvas_create(ui_wnd_rdesktop);
    // _rdesktop_canvas = lv_canvas_create(lv_scr_act());
    lv_obj_set_size(_rdesktop_canvas, wnd_width, wnd_height);
    /* Bind the off-screen buffer to the canvas.
       LV_IMG_CF_TRUE_COLOR is used for a full color image. */
    lv_canvas_set_buffer(_rdesktop_canvas, _rdesktop_fb, wnd_width, wnd_height, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(_rdesktop_canvas, LV_ALIGN_CENTER, 0, 0);

    // On success
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
    if(_rdesktop_fb)
    {
        free(_rdesktop_fb);
        _rdesktop_fb = NULL;
    }

    _rdesktop_fb_size = 0;
    wnd_width = 0;
    wnd_height = 0;
}

void* wnd_rdesktop_get_fb(void)
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
    if(_rdesktop_canvas)
    {
        lv_obj_invalidate(_rdesktop_canvas);
    }
}

void wnd_rdesktop_refresh_dirty(int x, int y, int w, int h)
{
    if(_rdesktop_canvas)
    {
        lv_area_t dirty;
        dirty.x1 = x;               /* left */
        dirty.y1 = y;               /* top */
        dirty.x2 = x + w - 1;       /* right  */
        dirty.y2 = y + h - 1;       /* bottom */

        lv_obj_invalidate_area(_rdesktop_canvas, &dirty);
    }
}