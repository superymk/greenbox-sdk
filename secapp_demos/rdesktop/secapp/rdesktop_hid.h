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