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

extern void sp_kbd_listener(struct kbd_key_event *event);
extern void sp_mouse_listener(struct mouse_event *event);

SAP_STATUS wnd_rdesktop_canvas_init(uint32_t in_wnd_width, uint32_t in_wnd_height)
{
    // libdtm_hid_register_kbd_listener(sp_kbd_listener);
    // libdtm_hid_register_mouse_listener(sp_mouse_listener);

    // libdtm_hid_register_kbd_listener(libgui_kbd_listener);
    // libdtm_hid_register_mouse_listener(libgui_mouse_listener);

    return wnd_rdesktop_create(in_wnd_width, in_wnd_height);
}

void wnd_rdesktop_canvas_fini(void)
{
    // libdtm_hid_unregister_kbd_listener(sp_kbd_listener);
    // libdtm_hid_unregister_mouse_listener(sp_mouse_listener);

    // libdtm_hid_unregister_kbd_listener(libgui_kbd_listener);
    // libdtm_hid_unregister_mouse_listener(libgui_mouse_listener);

    wnd_rdesktop_destroy();
}