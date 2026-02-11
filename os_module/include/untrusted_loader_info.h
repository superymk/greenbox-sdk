// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_UNTRUSTED_LOADER_INFO_H
#define SECAGENT_UNTRUSTED_LOADER_INFO_H

#ifndef __ASSEMBLY__

/// @brief Store a value for any NS process
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success
/// @return E_INV_ARG on invalid argument
extern OSSTATUS do_CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_SET(struct tz_cmd_packet *cmd_packet);

/// @brief Retrieve a value for any NS process
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success
/// @return E_INV_ARG on invalid argument
extern OSSTATUS do_CMD_NSDOM_SHARE_DATA_TO_ANY_NS_PROCESS_GET(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_UNTRUSTED_LOADER_INFO_H
