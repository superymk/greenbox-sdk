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

#ifndef _I965_REGS_H
#define _I965_REGS_H

#define RENDER_RING_BASE            0x02000
#define BSD_RING_BASE               0x04000
#define VIDEO_CS0_RING_BASE         0x12000
#define VIDEO_ECS_RING_BASE         0x1A000
#define VIDEO_CS1_RING_BASE         0x1C000
#define BLT_RING_BASE               0x22000

#define RING_CTL(base) ((base) + 0x3c)
#define     RB_ENABLE (1 << 0)

#define _HTOTAL_A                   0x60000
#define     HTOTAL_DISP_ACTIVE_PIXELS(htotal) (htotal) & 0xFFF
#define _VTOTAL_A                   0x6000C
#define     VTOTAL_DISP_ACTIVE_PIXELS(vtotal) (vtotal) & 0xFFF

#define HTOTAL(plane) (plane + _HTOTAL_A)
#define VTOTAL(plane) (plane + _VTOTAL_A)

#define _PIPECONF                   0x70008
#define     PIPE_ENABLE (1 << 31)
#define _DSPACNTR                   0x70180
#define     DISPLAY_PLANE_ENABLE (1 << 31)
#define     DISPLAY_PLANE_DISABLE 0

#define _DSPASURF                   0x7019C
#define _DSPASURFLIVE               0x701AC

#define _PIPEASRC                   0x6001C
#define     PIPE_H_SOURCE_IMAGE_SZ(src) ((src) >> 16) & 0xFFF
#define     PIPE_V_SOURCE_IMAGE_SZ(src) (src) & 0xFFF

#define DSPCNTR(plane)              (plane + _DSPACNTR)
#define DSPSURF(plane)              (plane + _DSPASURF)
#define DSPSURFLIVE(plane)          (plane + _DSPASURFLIVE)
#define PIPECONF(plane)             (plane + _PIPECONF)
#define PIPESRC(plane)              (plane + _PIPEASRC)

/* Sprite A control */
#define _DVSACNTR                   0x72180
#define     DVS_ENABLE (1 << 31)
#define     DVS_GAMMA_ENABLE (1 << 30)
#define     DVS_YUV_CORRECTION_ENABLE (1 << 28)
#define     DVS_PIXFORMAT_MASK (3 << 25)
// #define   	DVS_FORMAT_YUV422				(0<<25)
// #define   	DVS_FORMAT_RGBX2101010			(1<<25)
// #define   	DVS_FORMAT_RGBX8888				(2<<25)
// #define   	DVS_FORMAT_RGBX16161616			(3<<25)
#define         DVS_FORMAT_YUV422 (0 << 25)
#define         DVS_FORMAT_BGRX8888 (2 << 25)
#define     DVS_PIPE_CSC_ENABLE (1 << 24)
#define     DVS_SOURCE_KEY_ENABLE (1 << 22)
#define     DVS_RGB_ORDER_XBGR (1 << 20)

#define     DVS_YUV_RGB_CSC_DIS (1 << 19)
#define     DVS_YUV_REG_CSC_FORMAT (1 << 18)

#define     DVS_YUV_BYTE_ORDER_MASK (3 << 16)
#define     DVS_YUV_ORDER_YUYV (0 << 16)
#define     DVS_YUV_ORDER_UYVY (1 << 16)
#define     DVS_YUV_ORDER_YVYU (2 << 16)
#define     DVS_YUV_ORDER_VYUY (3 << 16)
#define     DVS_DEST_KEY (1 << 2)
#define     DVS_TRICKLE_FEED_DISABLE (1 << 14)
#define     DVS_ROTATE180 (1 << 15)
#define     DVS_SPRITE_GAMMA_DISABLE (1 << 13)
#define     DVS_TILED (1 << 10)
#define     DVS_DESTINATION_KEY (1 << 2)
#define _DVSALINOFF                 0x72184
#define _DVSASTRIDE                 0x72188
#define     DVS_STRIDE (1 << 6)
#define     DVS_STRIDE_RESERVE_MASK 0x3F
#define _DVSAPOS                    0x7218c
#define _DVSASIZE                   0x72190
#define     DVSSIZE_HEIGHT(dvssize) ((dvssize) >> 16) & 0xFFF
#define     DVSSIZE_HWIDTH(dvssize) (dvssize) & 0xFFF

#define _DVSSTRIDE_LINEAR_MEM_ALIGNMENT (64)

#define _DVSAKEYVAL                 0x72194
#define _DVSAKEYMSK                 0x72198
#define _DVSASURF                   0x7219c
#define _DVSAKEYMAXVAL              0x721a0
#define _DVSATILEOFF                0x721a4
#define _DVSASURFLIVE               0x721ac
#define _DVSASCALE                  0x72204
#define     DVS_SCALE_ENABLE (1 << 31)
#define     DVS_FILTER_MASK (3 << 29)
#define     DVS_FILTER_MEDIUM (0 << 29)
#define     DVS_FILTER_ENHANCING (1 << 29)
#define     DVS_FILTER_SOFTENING (2 << 29)
#define     DVS_VERTICAL_OFFSET_HALF (1 << 28) /* must be enabled below */
#define     DVS_VERTICAL_OFFSET_ENABLE (1 << 27)
#define _DVSAGAMC                   0x72300

#define _DVSBCNTR                   0x73180
#define _DVSBLINOFF                 0x73184
#define _DVSBSTRIDE                 0x73188
#define _DVSBPOS                    0x7318c
#define _DVSBSIZE                   0x73190
#define _DVSBKEYVAL                 0x73194
#define _DVSBKEYMSK                 0x73198
#define _DVSBSURF                   0x7319c
#define _DVSBKEYMAXVAL              0x731a0
#define _DVSBTILEOFF                0x731a4
#define _DVSBSURFLIVE               0x731ac
#define _DVSBSCALE                  0x73204
#define _DVSBGAMC                   0x73300

/* Cursor A control */
#define ILK_CURACNTR                0x70080
#define     CURCNTR_DISABLE_CURSOR_MASK ~((1 << 5) | 0x7)
#define _CURAPOS                    0x70088
#define ILK_CURBCNTR                0x700C0

#define _CURACNTR                   0x70080
#define CURCNTR(plane)              (plane + _CURACNTR)

/* Observation Architecture Control */
#define _OACONTROL                  0x2360
#define OA_PERF_ENABLE (1 << 0)

/* FB Compression Control */
#define _FBC_CONTROL                0x43208
#define     FBC_ENABLE (1 << 31)

/* VGA Control */
#define _VGACNTRL                   0x41000
#define     VGA_ENABLE (0 << 31)
#define     VGA_DISABLE (1 << 31)

#ifndef __ASSEMBLY__

typedef enum
{
    I965_PLANE_A = 0,
    I965_PLANE_B = 1,
    I965_PLANE_C = 2,
} ilk_plane_t;

#define I965_PLANE_A_OFFSET ((I965_PLANE_A) << 12)
#define I965_PLANE_B_OFFSET ((I965_PLANE_B) << 12)
#define I965_PLANE_C_OFFSET ((I965_PLANE_C) << 12)

#endif // __ASSEMBLY__
#endif // _I965_REGS_H