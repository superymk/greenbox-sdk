// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBDRV_HID_LISTENER_MGR_H
#define LIBDRV_HID_LISTENER_MGR_H

#include <drv/hid/input_events.h>


#ifndef __ASSEMBLY__

typedef void (*KBD_LISTENER)(struct kbd_key_event* event);
typedef void (*MOUSE_LISTENER)(struct mouse_event* event);


extern SAP_STATUS hid_listener_mgr_init(void);
extern void hid_listener_mgr_fini(void);

/// @brief Dispatch all queued keyboard events.
/// @param called_by_secsrv Set to true if the caller is a SecService (not a SecApp). If the caller is a SecService, 
/// this function disables all interrupts to prevent deadlock.
/// If the caller is a SecApp, the caller needs to prevent deadlock by itself.
extern void libdrv_hid_dispatch_kbd_events(bool called_by_secsrv);

/// @brief Dispatch all queued keyboard events.
/// @param called_by_secsrv Set to true if the caller is a SecService (not a SecApp). If the caller is a SecService, 
/// this function disables all interrupts to prevent deadlock.
/// If the caller is a SecApp, the caller needs to prevent deadlock by itself. 
extern void libdrv_hid_dispatch_mouse_events(bool called_by_secsrv);

extern SAP_STATUS libdrv_hid_register_kbd_listener(KBD_LISTENER listener);
extern SAP_STATUS libdrv_hid_unregister_kbd_listener(KBD_LISTENER listener);
extern SAP_STATUS libdrv_hid_register_mouse_listener(MOUSE_LISTENER listener);
extern SAP_STATUS libdrv_hid_unregister_mouse_listener(MOUSE_LISTENER listener);

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_LISTENER_MGR_H