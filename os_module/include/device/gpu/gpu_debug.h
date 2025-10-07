// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef GPU_DEBUG_H
#define GPU_DEBUG_H

#include <device/gpu/gpu_utils.h>
#include <device/gpu/intel/i965_regs.h>


#ifndef __ASSEMBLY__


OSSTATUS do_CMD_IOSM_FB_TEST_ON(struct tz_cmd_packet *cmd_packet);
OSSTATUS do_CMD_IOSM_FB_TEST_OFF(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // GPU_DEBUG_H