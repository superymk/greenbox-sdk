// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _DM_SECPROCESS_CONTROL_H
#define _DM_SECPROCESS_CONTROL_H

#include <app_sp_intfs.h>

#ifndef __ASSEMBLY__


extern int secapp_load(SAP_CONFIG* sap_cfg, SP_USTRUCT** out_params);
extern void secapp_unload(void);

extern int secapp_run(void);
extern void secapp_exit(void);


#endif // __ASSEMBLY__

#endif // _DM_SECPROCESS_CONTROL_H