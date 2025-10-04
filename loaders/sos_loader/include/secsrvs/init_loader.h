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