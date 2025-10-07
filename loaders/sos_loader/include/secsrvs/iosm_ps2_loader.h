// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOSM_PS2_LOADER_H
#define IOSM_PS2_LOADER_H

#include <appddk.h>

#ifndef __ASSEMBLY__

int _iosm_ps2_load(SL_CFG *sos_cfg);

int _iosm_ps2_run(void);

void iosm_ps2_terminate(void);

int _iosm_ps2_unload(void);


#endif // __ASSEMBLY__
#endif // IOSM_PS2_LOADER_H