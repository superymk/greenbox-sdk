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

#ifndef APP_SP_HID_BRIDGE_DEFS_H
#define APP_SP_HID_BRIDGE_DEFS_H

#include "hid_input_events.h"

#ifndef __ASSEMBLY__

typedef enum
{
	HID_DEV_INVALID = 0,
	HID_KEYBOARD = 1,
	HID_MOUSE = 2
} HID_DEV;

struct hid_event
{
	HID_DEV hid_dev;
	union
	{
		struct kbd_key_event kbd_event;
		struct mouse_event mouse_event;
	};
};

#endif // __ASSEMBLY__
#endif // APP_SP_HID_BRIDGE_DEFS_H