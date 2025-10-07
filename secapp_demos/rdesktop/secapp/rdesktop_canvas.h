// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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