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