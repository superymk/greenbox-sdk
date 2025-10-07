// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef INIT_SP_LOADER_H
#define INIT_SP_LOADER_H

#include <appddk.h>

#ifndef __ASSEMBLY__

int _init_sp_load(SL_CFG *sos_cfg);

int _init_sp_run(void);

/// @brief Unregister and unload all SecServices
/// Requirement: Invoke this function after SecOS exits
/// @param  
/// @return 
int init_sp_unload(void);


#endif // __ASSEMBLY__
#endif // INIT_SP_LOADER_H