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

extern void ui_init(void);
extern lv_obj_t * ui_ScreenLogin;
extern lv_obj_t * ui_ScreenMain;

static lv_obj_t* _frm_current = NULL;

void wnd_set_current(lv_obj_t* frm)
{
    _frm_current = frm;
}

lv_obj_t* wnd_get_current(void)
{
    return _frm_current;
}

void wnd_switch_to(lv_obj_t* frm)
{
    wnd_set_current(frm);
    lv_disp_load_scr(frm);
}

void wnd_init_main(void)
{
    wnd_set_current(ui_ScreenMain);
    ui_init();
}

void wnd_destroy_ScreenLogin(void)
{
    lv_obj_del_async(ui_ScreenLogin);
}