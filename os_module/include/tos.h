// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef GREENBOX_USE_UXMHF64_V1

#ifndef TINYOS_FUNCS_H
#define TINYOS_FUNCS_H

#ifndef __ASSEMBLY__

extern OSSTATUS sb_get_tos_info(NS_HC_RESP_GET_TINYOS_INFO *tos_info);

/// @brief Get TinyOS's configurations; e.g., LAPIC ID of the CPU running TinyOS, TinyOS is running or not.
/// @param out_is_running
/// @return 
extern OSSTATUS tos_probe_configs(bool *out_is_running);

/// @brief Return true if TinyOS is running.
/// @param out_is_running 
/// @return 
extern OSSTATUS tos_get_running_status(bool *out_is_running);

/// @brief Send an inter-processor-interrupt (IPI) to TinyOS. The caller must invoke the function <tos_probe_configs> 
/// first to initialize the IPI.
/// @param  
/// @return 
extern OSSTATUS tos_notify_ipi(void);

#endif // __ASSEMBLY__
#endif // TINYOS_FUNCS_H

#endif // !GREENBOX_USE_UXMHF64_V1