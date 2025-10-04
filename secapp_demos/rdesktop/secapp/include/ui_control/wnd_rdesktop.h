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

#ifndef RDESKTOP_UI_WND_RDESKTOP_H
#define RDESKTOP_UI_WND_RDESKTOP_H

#include <lvgl/lvgl.h>

#ifndef __ASSEMBLY__

extern SAP_STATUS wnd_rdesktop_create(uint32_t in_wnd_width, uint32_t in_wnd_height);
extern void wnd_rdesktop_destroy(void);
extern void wnd_rdesktop_show(void);
extern void* wnd_rdesktop_get_fb(void);

extern void wnd_rdesktop_refresh_all(void);

extern void wnd_rdesktop_refresh_dirty(int x, int y, int w, int h);

extern SAP_STATUS wnd_rdesktop_canvas_init(uint32_t in_wnd_width, uint32_t in_wnd_height);
extern void wnd_rdesktop_canvas_fini(void);

#endif // __ASSEMBLY__
#endif // RDESKTOP_UI_WND_RDESKTOP_H