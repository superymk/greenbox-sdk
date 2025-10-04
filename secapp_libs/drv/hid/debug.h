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
 * Authors: Eric Li
 *          Miao Yu
 *          Virgil Gligor
 */

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
