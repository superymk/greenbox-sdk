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