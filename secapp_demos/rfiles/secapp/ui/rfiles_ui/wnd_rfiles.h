// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#pragma once
#include <lvgl/lvgl.h>

/* Public: build and show the main window, then present the login window */
void wnd_rfiles_show(void);

/* Optional helper if you need to programmatically close login from elsewhere */
void wnd_rfiles_close_login_if_open(void);

/* Finalize and destroy the main window and all its children (no leaks). */
void wnd_rfiles_close(void);

/// @brief Return the root of all windows.
lv_obj_t* wnd_rfiles_get_root(void);