// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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