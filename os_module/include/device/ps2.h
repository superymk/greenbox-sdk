// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SA_DEVICE_PS2_H
#define SA_DEVICE_PS2_H

#include <os_types.h>



#ifndef __ASSEMBLY__
extern OSSTATUS do_CMD_IOSM_PS2_DRAIN_BUFFERS(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__

#endif // SA_DEVICE_PS2_H