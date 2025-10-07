// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RDESKTOP_UI_WND_MGR_H
#define RDESKTOP_UI_WND_MGR_H

#include <lvgl/lvgl.h>

#ifndef __ASSEMBLY__

/// @brief Show the main window and set it as the current window
extern void wnd_init_main(void);

extern void wnd_set_current(lv_obj_t* wnd);
extern lv_obj_t* wnd_get_current(void);

/// @brief Set the target window <wnd> as current and switch to the window.
/// @param wnd 
extern void wnd_switch_to(lv_obj_t* wnd);

/// @brief Destroy the Login window.
extern void wnd_destroy_ScreenLogin(void);

#endif // __ASSEMBLY__
#endif // RDESKTOP_UI_WND_MGR_H