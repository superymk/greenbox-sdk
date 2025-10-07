// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOSM_PS2_COMM_IOSM_H
#define IOSM_PS2_COMM_IOSM_H

#include <palddk.h>
#include <ps2_def.h>
#include <iosm_ps2_sp_intfs.h>

#ifndef __ASSEMBLY__

/// @brief Create Inter-SecProcess communication channels required by IOSM-PS2.
///
/// @return Return SUCCESS on succss.
///         Return EINTERNAL on errors.
extern SAP_STATUS comm_iosm_ps2_init(spid_t in_iosm_ps2_spid);
extern void comm_iosm_ps2_fini(void);

/// @brief Register the caller SecProcess's PS2 driver in IOSM-PS2.
extern SAP_STATUS iosm_ps2_register_ps2drv(void);

/// @brief Unregister the caller SecProcess's PS2 driver in IOSM-PS2.
extern SAP_STATUS iosm_ps2_unregister_ps2drv(void);

#endif // __ASSEMBLY__
#endif // IOSM_PS2_COMM_IOSM_H