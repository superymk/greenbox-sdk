// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Eric Li (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBDRV_HID_DEBUG_H
#define LIBDRV_HID_DEBUG_H

#ifndef __ASSEMBLY__

#include <palddk.h>
#include <drv/hid/input_events.h>

/// @brief Enable/Disable HID event printing
/// @param new_status True if enabling printing.
extern void hid_event_print_set(bool new_status);

/// @brief Print keyboard event using printf.
/// @param driver name of the driver generating the event.
/// @param event pointer to event generated.
extern void print_kbd_key_event(const char *driver, const struct kbd_key_event *event);

/// @brief 
/// @param driver 
/// @param event 
extern void print_mouse_event(const char *driver, struct mouse_event* event);

#endif // __ASSEMBLY__
#endif // LIBDRV_HID_DEBUG_H
