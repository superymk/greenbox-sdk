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

#include <sap_headers.h>
#include <palddk.h>

#include "rdesktop_canvas.h"
#include "ui_control/wnd_rdesktop.h"

static int realWidth, realHeight, bytesPerPixel, rowStride;
static char *sdlPixels = NULL;

__attribute__((unused))
static uint32_t get(rfbClient *cl, int x, int y)
{
	switch (bytesPerPixel)
	{
	case 1:
		return ((uint8_t *)cl->frameBuffer)[x + y * cl->width];
	case 2:
		return ((uint16_t *)cl->frameBuffer)[x + y * cl->width];
	case 4:
		return ((uint32_t *)cl->frameBuffer)[x + y * cl->width];
	default:
		rfbClientErr("Unknown bytes/pixel: %d", bytesPerPixel);
		exit(1);
	}
}

__attribute__((unused))
static void put(int x, int y, uint32_t v)
{
	switch (bytesPerPixel)
	{
	case 1:
		((uint8_t *)sdlPixels)[x + y * rowStride] = v;
		break;
	case 2:
		((uint16_t *)sdlPixels)[x + y * rowStride] = v;
		break;
	case 4:
		((uint32_t *)sdlPixels)[x + y * rowStride] = v;
		break;
	default:
		rfbClientErr("Unknown bytes/pixel: %d", bytesPerPixel);
		exit(1);
	}
}

__attribute__((unused))
static void resizeRectangleToReal(rfbClient *cl, int x, int y, int w, int h)
{
	int i0 = x * realWidth / cl->width;
	int i1 = ((x + w) * realWidth - 1) / cl->width + 1;
	int j0 = y * realHeight / cl->height;
	int j1 = ((y + h) * realHeight - 1) / cl->height + 1;
	int i, j;

	for (j = j0; j < j1; j++)
		for (i = i0; i < i1; i++)
		{
			int x0 = i * cl->width / realWidth;
			int x1 = ((i + 1) * cl->width - 1) / realWidth + 1;
			int y0 = j * cl->height / realHeight;
			int y1 = ((j + 1) * cl->height - 1) / realHeight + 1;
			uint32_t r = 0, g = 0, b = 0;

			for (y = y0; y < y1; y++)
				for (x = x0; x < x1; x++)
				{
					uint32_t v = get(cl, x, y);

#define REDSHIFT cl->format.redShift
#define REDMAX cl->format.redMax
#define GREENSHIFT cl->format.greenShift
#define GREENMAX cl->format.greenMax
#define BLUESHIFT cl->format.blueShift
#define BLUEMAX cl->format.blueMax
					r += (v >> REDSHIFT) & REDMAX;
					g += (v >> GREENSHIFT) & GREENMAX;
					b += (v >> BLUESHIFT) & BLUEMAX;
				}
			r /= (x1 - x0) * (y1 - y0);
			g /= (x1 - x0) * (y1 - y0);
			b /= (x1 - x0) * (y1 - y0);

			put(i, j, (r << REDSHIFT) | (g << GREENSHIFT) | (b << BLUESHIFT));
		}
}

void rdesktop_canvas_update(rfbClient *cl, int x, int y, int w, int h)
{
	resizeRectangleToReal(cl, x, y, w, h);

	wnd_rdesktop_refresh_dirty(x, y, w, h);
	
	// w = ((x + w) * realWidth - 1) / cl->width + 1;
	// h = ((y + h) * realHeight - 1) / cl->height + 1;
	// x = x * realWidth / cl->width;
	// y = y * realHeight / cl->height;
	// w -= x;
	// h -= y;
	// SDL_UpdateRect(rfbClientGetClientData(cl, SDL_Init), x, y, w, h);
}

void rdesktop_canvas_init(int in_realWidth, int in_realHeight, int in_bytesPerPixel, int in_rowStride, char* in_sdlPixels)
{
	realWidth = in_realWidth;
	realHeight = in_realHeight; 
	bytesPerPixel = in_bytesPerPixel;
	rowStride = in_rowStride;
	sdlPixels = in_sdlPixels;
}