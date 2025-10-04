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

#ifndef IOSM_FB_LOADER_H
#define IOSM_FB_LOADER_H

#include <appddk.h>

#ifndef __ASSEMBLY__

int _iosm_fb_load(SL_CFG *sos_cfg);

int _iosm_fb_run(void);

void iosm_fb_terminate(void);

int _iosm_fb_unload(void);

#endif // __ASSEMBLY__
#endif // IOSM_FB_LOADER_H