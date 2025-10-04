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

#ifndef RDESKTOP_CANVAS_H
#define RDESKTOP_CANVAS_H

#include <sap_headers.h>
#include <palddk.h>

#include <rfb/rfbclient.h>

#ifndef __ASSEMBLY__

extern void rdesktop_canvas_init(int in_realWidth, int in_realHeight, int in_bytesPerPixel, int in_rowStride, char* in_sdlPixels);

extern void rdesktop_canvas_update(rfbClient *cl, int x, int y, int w, int h);

#endif // __ASSEMBLY__
#endif // RDESKTOP_CANVAS_H