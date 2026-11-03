// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef GREENBOX_USE_UXMHF64_V1

#ifndef TINYOS_FUNCS_H
#define TINYOS_FUNCS_H

#ifndef __ASSEMBLY__

/// @brief Get TinyOS info from SecBase
/// @param tos_info Output TinyOS info
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS sb_get_tos_info(NS_HC_RESP_GET_TINYOS_INFO *tos_info);

/// @brief Get TinyOS configurations (e.g., LAPIC ID, running status)
/// @param out_is_running Output running status
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS tos_probe_configs(bool *out_is_running);

/// @brief Check if TinyOS is running
/// @param out_is_running Output running status
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS tos_get_running_status(bool *out_is_running);

/// @brief Send an inter-processor interrupt (IPI) to TinyOS
/// The caller must invoke tos_probe_configs first to initialize the IPI.
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS tos_notify_ipi(void);

#endif // __ASSEMBLY__
#endif // TINYOS_FUNCS_H

#endif // !GREENBOX_USE_UXMHF64_V1