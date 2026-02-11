// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_UNTRUSTED_LOADER_UTILS_H
#define SECAGENT_UNTRUSTED_LOADER_UTILS_H

#ifndef __ASSEMBLY__

/// @brief Initialize NS domain utilities
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS nsdom_utils_init(void);

/// @brief Finalize NS domain utilities
extern void nsdom_utils_fini(void);

/// @brief Try to acquire a global spinlock
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success
/// @return E_INV_ARG on invalid argument
extern OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE(struct tz_cmd_packet *cmd_packet);

/// @brief Release a global spinlock
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success
/// @return E_INV_ARG on invalid argument
extern OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_UNTRUSTED_LOADER_UTILS_H
