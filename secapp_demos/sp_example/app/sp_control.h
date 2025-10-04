/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#ifndef _DM_SECPROCESS_CONTROL_H
#define _DM_SECPROCESS_CONTROL_H

#include <app_sp_intfs.h>

#ifndef __ASSEMBLY__


extern int secapp_load(SP_USTRUCT** out_params);
extern void secapp_unload(void);

extern int secapp_run(void);
extern void secapp_exit(void);


#endif // __ASSEMBLY__

#endif // _DM_SECPROCESS_CONTROL_H