// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SL_SECSERVICES_H
#define SL_SECSERVICES_H

#include <appddk.h>

#ifndef __ASSEMBLY__

/// @brief SecServices may need to perform certian initialization before SecOS running. For example, disabling XHCI is
/// much faster if done before running SecOS.
/// [NOTE] This function can only be called after loading SecServices.
/// @param sos_cfg
/// @return
extern SL_STATUS ld_secsrvs_init_before_secos_run(SL_CFG *sos_cfg);

/// @brief SecServices may need to perform certian finalization after SecOS termination.
/// [NOTE] This function can only be called before unloading SecServices.
/// @param sos_cfg 
/// @return 
extern SL_STATUS ld_secsrvs_fini_after_secos_terminate(SL_CFG *sos_cfg);

extern int secsrvs_load(SL_CFG *sos_cfg, uint32_t cpu_affinity);

/// @brief Run SecServices enabled in <sos_cfg>. This function does not terminate running SecServices if failing to boot
/// any SecServices.
/// @param sos_cfg 
/// @return 
extern SL_STATUS secsrvs_run(SL_CFG *sos_cfg);

/// @brief Unregister and unload all SecServices (except Init SecProcess)
/// @param sos_cfg 
extern void secsrvs_terminate(SL_CFG *sos_cfg);

/// @brief Unload all SecServices (except Init SecProcess)
/// @param sos_cfg 
extern void secsrvs_unload(SL_CFG *sos_cfg);

#endif // __ASSEMBLY__
#endif // SL_SECSERVICES_H