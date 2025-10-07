// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RDESKTOP_HID_H
#define RDESKTOP_HID_H

#include <sap_headers.h>
#include <palddk.h>

#include <rfb/rfbclient.h>

#ifndef __ASSEMBLY__

extern void rdesktop_hid_init(int in_realWidth, int in_realHeight, int in_bytesPerPixel, int in_rowStride, char* in_sdlPixels);

/// @brief Send mouse pointer position to the RFB server
/// @param cl 
/// @param pos_x 
/// @param pos_y 
extern void rfb_send_mouse_pos(rfbClient *cl, int pos_x, int pos_y);

extern void rdesktop_handle_kbd_event(rfbClient *cl, struct kbd_key_event *event);
extern void rdesktop_handle_mouse_event(rfbClient *cl, struct mouse_event *event);


#endif // __ASSEMBLY__
#endif // RDESKTOP_HID_H