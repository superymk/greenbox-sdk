// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_UNTRUSTED_LOADER_UTILS_H
#define SECAGENT_UNTRUSTED_LOADER_UTILS_H

extern OSSTATUS nsdom_utils_init(void);

extern void nsdom_utils_fini(void);

extern OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE(struct tz_cmd_packet *cmd_packet);
extern OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE(struct tz_cmd_packet *cmd_packet);

#endif // SECAGENT_UNTRUSTED_LOADER_UTILS_H
