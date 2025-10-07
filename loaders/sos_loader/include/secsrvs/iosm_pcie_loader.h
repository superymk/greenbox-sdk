// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOSM_PCIE_LOADER_H
#define IOSM_PCIE_LOADER_H

#include <appddk.h>

#ifndef __ASSEMBLY__

int _iosm_pcie_load(SL_CFG *sos_cfg);
int _iosm_pcie_run(void);
void iosm_pcie_terminate(void);
int iosm_pcie_unload(void);


#endif // __ASSEMBLY__
#endif // IOSM_PCIE_LOADER_H