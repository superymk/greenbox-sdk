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

#ifndef _ILK_REGS_H
#define _ILK_REGS_H

/* Sprite A control */
#define _DVSACNTR		0x72180
#define   DVS_ENABLE		(1<<31)
#define   DVS_GAMMA_ENABLE	(1<<30)
#define   DVS_YUV_CORRECTION_DISABLE	(1<<28)
#define   DVS_PIXFORMAT_MASK	(7<<25)
#define   	DVS_FORMAT_YUV422				(0<<25)
#define   	DVS_FORMAT_RGBX101010			(1<<25)
#define   	DVS_FORMAT_RGBX888				(2<<25)
#define   	DVS_FORMAT_RGBX161616			(3<<25)
#define 	DVS_FORMAT_YUV444				(4<<25)
#define		DVS_FORMAT_RGBEXTENDED101010	(5<<25)
#define   DVS_PIPE_CSC_ENABLE   (1<<24)
#define   DVS_SOURCE_KEY	(1<<22)
#define   DVS_RGB_ORDER_XBGR	(1<<20)

#define   DVS_YUV_RGB_CSC_DIS		(1<<19)
#define   DVS_YUV_REG_CSC_FORMAT	(1<<18)

#define   DVS_YUV_BYTE_ORDER_MASK (3<<16)
#define   DVS_YUV_ORDER_YUYV	(0<<16)
#define   DVS_YUV_ORDER_UYVY	(1<<16)
#define   DVS_YUV_ORDER_YVYU	(2<<16)
#define   DVS_YUV_ORDER_VYUY	(3<<16)
#define   DVS_DEST_KEY		(1<<2)
#define   DVS_TRICKLE_FEED_DISABLE (1<<14)
#define   DVS_ROTATE180	(1<<15)
#define	  DVS_SPRITE_GAMMA_DISABLE	(1<<13)
#define   DVS_TILED		(1<<10)
#define   DVS_DESTINATION_KEY	(1<<2)
#define _DVSALINOFF		0x72184
#define _DVSASTRIDE		0x72188
#define   	DVS_STRIDE		(1<<6)
#define		DVS_STRIDE_RESERVE_MASK		0x3F
#define _DVSAPOS		0x7218c
#define _DVSASIZE		0x72190
#define _DVSAKEYVAL		0x72194
#define _DVSAKEYMSK		0x72198
#define _DVSASURF		0x7219c
#define _DVSAKEYMAXVAL		0x721a0
#define _DVSATILEOFF		0x721a4
#define _DVSASURFLIVE		0x721ac
#define _DVSASCALE		0x72204
#define   DVS_SCALE_ENABLE	(1<<31)
#define   DVS_FILTER_MASK	(3<<29)
#define   DVS_FILTER_MEDIUM	(0<<29)
#define   DVS_FILTER_ENHANCING	(1<<29)
#define   DVS_FILTER_SOFTENING	(2<<29)
#define   DVS_VERTICAL_OFFSET_HALF (1<<28) /* must be enabled below */
#define   DVS_VERTICAL_OFFSET_ENABLE (1<<27)
#define _DVSAGAMC		0x72300

#define _DVSBCNTR		0x73180
#define _DVSBLINOFF		0x73184
#define _DVSBSTRIDE		0x73188
#define _DVSBPOS		0x7318c
#define _DVSBSIZE		0x73190
#define _DVSBKEYVAL		0x73194
#define _DVSBKEYMSK		0x73198
#define _DVSBSURF		0x7319c
#define _DVSBKEYMAXVAL		0x731a0
#define _DVSBTILEOFF		0x731a4
#define _DVSBSURFLIVE		0x731ac
#define _DVSBSCALE		0x73204
#define _DVSBGAMC		0x73300

#endif // _ILK_REGS_H