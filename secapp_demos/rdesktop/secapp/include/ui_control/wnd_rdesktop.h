// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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