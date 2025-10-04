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

#ifndef GPU_CONFIG_H
#define GPU_CONFIG_H

// Reserved GGTT Space Configuration
#define GPU_RESV_VRAM_START_GPAGE	0x0E400000    //   must be less than 0x10000000
		// [Ticket 102] GPU_RESV_VRAM_START_GPAGE is related to a performance bug
#define OVL_GVADDR_LAST_GPAGE		0x0EF00000 //  must be less than 0x10000000
#define SECAPP_RESV_GGTT_PAGES	((OVL_GVADDR_LAST_GPAGE - GPU_RESV_VRAM_START_GPAGE) >> PAGE_SHIFT_4K)



#endif // GPU_CONFIG_H
