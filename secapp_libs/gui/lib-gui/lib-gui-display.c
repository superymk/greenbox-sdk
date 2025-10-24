// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <gui/ui/libgui_headers.h>

static void *fbp = NULL; // FB ptr
static lv_display_t *disp = NULL;

//// Display configs
static uint32_t vinfo_xres = 0;
static uint32_t vinfo_yres = 0;
static uint32_t vinfo_xoffset = 0;
static uint32_t vinfo_yoffset = 0;
static uint32_t vinfo_bits_per_pixel = 0;
static uint32_t finfo_line_length = 0; // bytes for each line, aka "stride"

static void _libgui_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    if (!fbp)
    {
        lv_display_flush_ready(display);
        return;
    }

    // Reject totally off-screen
    if (area->x2 < 0 || area->y2 < 0 ||
        area->x1 >= (int32_t)vinfo_xres || area->y1 >= (int32_t)vinfo_yres)
    {
        lv_display_flush_ready(display);
        return;
    }

    // Clamp to screen
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > (int32_t)vinfo_xres - 1 ? (int32_t)vinfo_xres - 1 : area->x2;
    int32_t act_y2 = area->y2 > (int32_t)vinfo_yres - 1 ? (int32_t)vinfo_yres - 1 : area->y2;

    int32_t dst_w = act_x2 - act_x1 + 1;
    int32_t dst_h = act_y2 - act_y1 + 1;
    if (dst_w <= 0 || dst_h <= 0)
    {
        lv_display_flush_ready(display);
        return;
    }

    // Source stride is the rendered-area width (NOT screen width)
    const int32_t src_w = (area->x2 - area->x1 + 1);
    const int32_t off_x = act_x1 - area->x1;
    const int32_t off_y = act_y1 - area->y1;

    // ARGB8888, little-endian memory, 4 bytes per pixel
    const uint8_t bpp = 32;
    const int32_t dst_pitch_bytes = (int32_t)finfo_line_length; // bytes per FB row (your stride = width*4)
    uint8_t *fb_base = (uint8_t *)fbp;

    // Start of the source row inside LVGL’s draw buffer
    const lv_color32_t *src_row = (const lv_color32_t *)px_map + off_y * src_w + off_x;

    for (int32_t y = 0; y < dst_h; y++)
    {
        // Destination row in framebuffer
        uint8_t *dst = fb_base + (size_t)(vinfo_yoffset + act_y1 + y) * dst_pitch_bytes + (size_t)(vinfo_xoffset + act_x1) * (bpp / 8);

        // memcpy one row (ARGB8888 → ARGB8888)
        memcpy(dst, src_row, (size_t)dst_w * 4);

        // advance to next LVGL row
        src_row += src_w;
    }

    lv_display_flush_ready(display);
}

/********* Public Functions *********/
SAP_STATUS libgui_display_init(struct libgui_config *cfg)
{
    if (!cfg)
        return EINVAL;
    if (cfg->fb_vaddr == INVALID_VADDR)
        return EINVAL;
    if (cfg->fb_width == 0 || cfg->fb_height == 0)
        return EINVAL;
    if (disp)
        return EINVAL; // already initialized

    // 1) Create LVGL display with your screen size
    disp = lv_display_create((lv_coord_t)cfg->fb_width, (lv_coord_t)cfg->fb_height);
    if (!disp)
        return EINVAL;

    // ARGB8888 throughout (LVGL draw buffer format)
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);

    // 2) Attach your flush callback (this WILL copy from s_drawbuf -> fb)
    lv_display_set_flush_cb(disp, _libgui_flush_cb);

    // 3) Allocate a separate draw buffer for PARTIAL mode
    //      Choose a reasonable number of rows to balance RAM vs speed.
    //      Here: up to 64 rows, but never more than screen height.
    uint32_t rows = (cfg->fb_height >= 64u) ? 64u : cfg->fb_height;
    size_t drawbuf_bytes = (size_t)cfg->fb_width * rows * sizeof(lv_color32_t);

    // Use LVGL’s allocator so LVGL can track it (you may use lv_malloc or lv_mem_alloc depending on your setup)
    void *s_drawbuf = lv_malloc(drawbuf_bytes);
    if (!s_drawbuf)
        return EINVAL;

    // 4) Register the draw buffer with PARTIAL render mode
    //      - buf1: s_drawbuf (temporary render surface)
    //      - buf2: NULL (single buffer is fine for PARTIAL)
    //      - size: BYTES
    //      - mode: PARTIAL (LVGL fills s_drawbuf for dirty areas; your flush copies to real FB)
    lv_display_set_buffers(disp, s_drawbuf, NULL, drawbuf_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 5) (Optional) Make the active screen transparent (if you composite)
    lv_obj_set_style_bg_opa(lv_display_get_screen_active(disp), LV_OPA_TRANSP, 0);

    // 6) Keep handy FB info for the flush callback
    fbp = (void *)cfg->fb_vaddr;
    vinfo_xres = cfg->fb_width;
    vinfo_yres = cfg->fb_height;
    vinfo_xoffset = 0;
    vinfo_yoffset = 0;
    vinfo_bits_per_pixel = 32;          // ARGB8888
    finfo_line_length = vinfo_xres * 4; // stride in BYTES

    // 7) Make this display default (optional)
    lv_display_set_default(disp);

    return SUCCESS;
}

void libgui_display_fini(void)
{
    lv_deinit();

    fbp = NULL;
    disp = NULL;

    //// Display configs
    vinfo_xres = 0;
    vinfo_yres = 0;
    vinfo_xoffset = 0;
    vinfo_yoffset = 0;
    vinfo_bits_per_pixel = 0;
    finfo_line_length = 0;
}