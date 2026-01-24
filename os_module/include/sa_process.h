// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_PROCESS_H
#define SECAGENT_PROCESS_H

#ifndef __ASSEMBLY__

/// @brief Register SecProcess memory sections
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_REGISTER_SECPROCESS_MEMORY(struct tz_cmd_packet *cmd_packet);

/// @brief Register SecProcess
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_REGISTER_SECPROCESS(struct tz_cmd_packet *cmd_packet);

/// @brief Run SecProcess
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_RUN_SECPROCESS(struct tz_cmd_packet *cmd_packet);

/// @brief Soft terminate SecProcess
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_SOFT_TERMINATE_SECPROCESS(struct tz_cmd_packet *cmd_packet);

/// @brief Force terminate SecProcess
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_FORCE_TERMINATE_SECPROCESS(struct tz_cmd_packet *cmd_packet);

/// @brief Notify SecProcess
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_NOTIFY_SECPROCESS(struct tz_cmd_packet *cmd_packet);

/********* SecProcess Information Query *********/
/// @brief Get SecProcess IDs and names
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_SECPROCESS_GET_IDS_AND_NAMES(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_PROCESS_H