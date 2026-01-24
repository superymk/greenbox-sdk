// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECOS_FUNCS_H
#define SECOS_FUNCS_H

#ifndef __ASSEMBLY__

/// @brief Return the status of SecOS
/// @param out_status Output status
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS sos_get_status(VKSTATUS* out_status);

/// @brief Run SecOS
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_RUN_SECOS(struct tz_cmd_packet *cmd_packet);
// extern OSSTATUS do_CMD_RESUME_SECOS(struct tz_cmd_packet *cmd_packet);

/// @brief Probe SecOS status
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_PROBE_SECOS_STATUS(struct tz_cmd_packet *cmd_packet);

/// @brief Prepare SecOS registration
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_REGISTER_SECOS_PREPARE(struct tz_cmd_packet *cmd_packet);

/// @brief Register SecOS
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_REGISTER_SECOS(struct tz_cmd_packet *cmd_packet);

/// @brief Unregister SecOS
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_UNREGISTER_SECOS(struct tz_cmd_packet *cmd_packet);

/// @brief Terminate SecOS
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_TERMINATE_SECOS(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECOS_FUNCS_H