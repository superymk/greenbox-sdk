// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_INTR_H
#define SECAGENT_INTR_H
#include <os_types.h>


#ifndef __ASSEMBLY__

/// @brief Trigger OS interrupt
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_OS_TRIGGER_INTR(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_INTR_H