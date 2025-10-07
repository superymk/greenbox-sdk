// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
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