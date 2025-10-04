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

#ifndef RDESKTOP_UI_WND_LOGIN_H
#define RDESKTOP_UI_WND_LOGIN_H

#include <lvgl/lvgl.h>

#ifndef __ASSEMBLY__

/// @brief Use this function if RDesktop SecApp does not use LVGL to display the remote desktop (though initialized all
/// windows in <wnd_init_main>).
extern void wnd_login_show(void);

/// @brief Invoke this function if the caller needs to pop up the login window again for the first time (i.e., not
/// re-trying the VNC connection).
extern void wnd_login_mark_first_load(void);

/// @brief Invoke this function if the caller needs to pop up the login window again due to re-trying the VNC connection.
extern void wnd_login_unmark_first_load(void);




/*********** Temp Functions **********/
/// @brief Sleep for the given seconds when showing the login window while not pausing the LVGL GUI.
extern void _temp_wait_in_wnd_login(const char *error_msg, uint32_t sleep_ms);

#endif // __ASSEMBLY__
#endif // RDESKTOP_UI_WND_LOGIN_H