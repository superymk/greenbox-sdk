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

static void _libgui_flush_cb(lv_display_t * display, const lv_area_t *area, uint8_t * px_map)
{
    lv_color32_t *color_p = (lv_color32_t *)px_map;

    if (fbp == NULL ||
        area->x2 < 0 ||
        area->y2 < 0 ||
        area->x1 > (int32_t)vinfo_xres - 1 ||
        area->y1 > (int32_t)vinfo_yres - 1)
    {
        lv_display_flush_ready(display);
        return;
    }

    // Truncate the area to the screen
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > (int32_t)vinfo_xres - 1 ? (int32_t)vinfo_xres - 1 : area->x2;
    int32_t act_y2 = area->y2 > (int32_t)vinfo_yres - 1 ? (int32_t)vinfo_yres - 1 : area->y2;

    lv_coord_t w = (act_x2 - act_x1 + 1);
    long int location = 0;
    long int byte_location = 0;
    unsigned char bit_location = 0;

    // 32 or 24 bit per pixel
    if (vinfo_bits_per_pixel == 32 || vinfo_bits_per_pixel == 24)
    {
        uint32_t *fbp32 = (uint32_t *)fbp;
        int32_t y;

        {
            location = (act_x1 + vinfo_xoffset) + (act_y1 + vinfo_yoffset) * finfo_line_length / 4;
            // printf("[Superymk] <_libgui_flush_cb> act_x1:%d, act_x2:%d, act_y1:%d, act_y2:%d, location_start:%ld\n",
            //     act_x1, act_x2, act_y1, act_y2, location);
        }

        for (y = act_y1; y <= act_y2; y++)
        {
            location = (act_x1 + vinfo_xoffset) + (y + vinfo_yoffset) * finfo_line_length / 4;
            memcpy(&fbp32[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1) * 4);
            
            color_p = ((lv_color32_t *)color_p) + w;
        }
    }
    // 16 bit per pixel
    else if (vinfo_bits_per_pixel == 16)
    {
        uint16_t *fbp16 = (uint16_t *)fbp;
        int32_t y;
        for (y = act_y1; y <= act_y2; y++)
        {
            location = (act_x1 + vinfo_xoffset) + (y + vinfo_yoffset) * finfo_line_length / 2;
            memcpy(&fbp16[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1) * 2);

            color_p = ((lv_color16_t *)color_p) + w;
        }
    }
    // // 8 bit per pixel
    // else if (vinfo_bits_per_pixel == 8)
    // {
    //     uint8_t *fbp8 = (uint8_t *)fbp;
    //     int32_t y;
    //     for (y = act_y1; y <= act_y2; y++)
    //     {
    //         location = (act_x1 + vinfo_xoffset) + (y + vinfo_yoffset) * finfo_line_length;
    //         memcpy(&fbp8[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1));
    //         color_p += w;
    //     }
    // }
    // // 1 bit per pixel
    // else if (vinfo_bits_per_pixel == 1)
    // {
    //     uint8_t *fbp8 = (uint8_t *)fbp;
    //     int32_t x;
    //     int32_t y;
    //     for (y = act_y1; y <= act_y2; y++)
    //     {
    //         for (x = act_x1; x <= act_x2; x++)
    //         {
    //             location = (x + vinfo_xoffset) + (y + vinfo_yoffset) * vinfo_xres;
    //             byte_location = location / 8; /* find the byte we need to change */
    //             bit_location = location % 8;  /* inside the byte found, find the bit we need to change */
    //             fbp8[byte_location] &= ~(((uint8_t)(1)) << bit_location);
    //             fbp8[byte_location] |= ((uint8_t)(color_p->full)) << bit_location;
    //             color_p++;
    //         }

    //         color_p += area->x2 - act_x2;
    //     }
    // }
    else
    {
        /*Not supported bit per pixel*/
    }

    // May be some direct update command is required
    // ret = ioctl(state->fd, FBIO_UPDATE, (unsigned long)((uintptr_t)rect));

    lv_display_flush_ready(display);

    // int32_t x;
    // int32_t y;
    // lv_coord_t hres = disp_drv->rotated == 0 ? disp_drv->hor_res : disp_drv->ver_res;
    // lv_coord_t vres = disp_drv->rotated == 0 ? disp_drv->ver_res : disp_drv->hor_res;

    // uint32_t w = lv_area_get_width(area);
    // for(y = area->y1; y <= area->y2 && y < disp_drv->ver_res; y++) {
    //     memcpy(&((char*)fbp)[y * vinfo_xres + area->x1], color_p, w * sizeof(lv_color_t));
    //     color_p += w;
    // }

    // lv_disp_flush_ready(disp_drv);

    // if(fbp == NULL)
    // {
    //     lv_disp_flush_ready(disp_drv);
    //     return;
    // }

    // memcpy(fbp, color_p, vinfo_xres * vinfo_yres * 4);
    // lv_disp_flush_ready(disp_drv);
}


/********* Public Functions *********/
SAP_STATUS libgui_display_init(struct libgui_config *cfg)
{
    SAP_STATUS status = SUCCESS;

    // Check: <cfg> must be valid
    if (!cfg)
        return EINVAL;

    if (cfg->fb_vaddr == INVALID_VADDR)
        return EINVAL;

    // Check: No double init
    if (disp)
        return EINVAL;

    // Initialize window canvas
    lv_display_t *disp = lv_display_create(cfg->fb_width, cfg->fb_height);
    lv_display_set_flush_cb(disp, _libgui_flush_cb);

    // size must be in BYTES and must match full screen in DIRECT mode
    size_t bytes = (size_t)cfg->fb_width * cfg->fb_height * sizeof(lv_color32_t);
    lv_display_set_buffers(disp,
                           (void *)cfg->fb_vaddr,   // buf1 (your FB)
                           NULL,                    // buf2 (optional)
                           bytes,                   // BYTES, not pixels
                           LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);

    lv_display_set_default(disp);

    // Make the active screen transparent
    lv_obj_set_style_bg_opa(lv_display_get_screen_active(disp), LV_OPA_TRANSP, 0);

    //// Display configs
    vinfo_xres = cfg->fb_width;
    vinfo_yres = cfg->fb_height;
    vinfo_xoffset = 0;
    vinfo_yoffset = 0;
    vinfo_bits_per_pixel = 32;
    finfo_line_length = vinfo_xres * 4; // bytes for each line, aka "stride"

    // On success
    return SUCCESS;





//     // Initialize a descriptor for the buffer
//     static lv_disp_draw_buf_t disp_buf;
//     lv_disp_draw_buf_init(&disp_buf, (void *)cfg->fb_vaddr, NULL, cfg->fb_width * cfg->fb_height);

//     // Initialize and register a display driver
//     static lv_disp_drv_t disp_drv;
//     lv_disp_drv_init(&disp_drv);

//     disp_drv.full_refresh = 0;
//     disp_drv.direct_mode = 1;

//     disp_drv.draw_buf = &disp_buf;
//     disp_drv.flush_cb = libgui_flush_cb;
//     disp_drv.hor_res = cfg->fb_width;
//     disp_drv.ver_res = cfg->fb_height;
//     disp = lv_disp_drv_register(&disp_drv);
//     if (!disp)
//     {
//         GUI_ERR("Init Lib-GUI error: Failed to init display!\n");
//         status = EINTERNAL;
//         goto init_disp_err;
//     }

//     lv_disp_set_bg_opa(NULL, LV_OPA_TRANSP);

//     //// Display configs
//     vinfo_xres = cfg->fb_width;
//     vinfo_yres = cfg->fb_height;
//     vinfo_xoffset = 0;
//     vinfo_yoffset = 0;
//     vinfo_bits_per_pixel = 32;
//     finfo_line_length = vinfo_xres * 4; // bytes for each line, aka "stride"



    



//     GUI_INFO("Initialize Lib-GUI Success\n");
//     return SUCCESS;

// init_cursor_err:
// init_mouse_err:
// init_kbd_err:
// init_disp_err:
//     return status;
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

