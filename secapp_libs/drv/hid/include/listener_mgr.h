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