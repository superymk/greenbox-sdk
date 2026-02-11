// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_OPS_HEADER_H
#define SECAGENT_OPS_HEADER_H

/// @brief Global OS function table
extern struct os_funcs *os;

/// @brief Mark SecAgent as running
extern void secagent_mark_running(void);

/// @brief Mark SecAgent as not running
extern void secagent_mark_not_running(void);

/// @brief Check if SecAgent is running
/// @return true if running, false otherwise
extern bool secagent_is_running(void);

/// @brief Initialize SecAgent
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS secagent_init(void);

/// @brief Finalize SecAgent
extern void secagent_fini(void);

/// @brief Initialize OS function table
extern void os_init(void);

/// @brief Finalize OS function table
extern void os_fini(void);

#ifndef GREENBOX_USE_UXMHF64_V1
/// @brief Boot TinyOS via SecBase
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
OSSTATUS do_CMD_SECBASE_BOOT_TINYOS(struct tz_cmd_packet* cmd_packet);
#endif // !GREENBOX_USE_UXMHF64_V1

/// @brief Initialize SecBase per CPU on-demand
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_SECBASE_INIT_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet);

/// @brief Finalize SecBase per CPU on-demand
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_SECBASE_FINI_ONDEMAND_PER_CPU(struct tz_cmd_packet *cmd_packet);

/// @brief Check SecAgent presence
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_SECAGENT_PRESENCE_REQ(struct tz_cmd_packet *cmd_packet);

#endif // SECAGENT_OPS_HEADER_H
