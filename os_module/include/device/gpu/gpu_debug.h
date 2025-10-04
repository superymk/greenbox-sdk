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

#ifndef GPU_DEBUG_H
#define GPU_DEBUG_H

#include <device/gpu/gpu_utils.h>
#include <device/gpu/intel/i965_regs.h>


#ifndef __ASSEMBLY__


OSSTATUS do_CMD_IOSM_FB_TEST_ON(struct tz_cmd_packet *cmd_packet);
OSSTATUS do_CMD_IOSM_FB_TEST_OFF(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // GPU_DEBUG_H