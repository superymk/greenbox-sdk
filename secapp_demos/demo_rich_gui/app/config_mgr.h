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

#ifndef _CONFIG_MGR_H
#define _CONFIG_MGR_H

#include <app_sp_intfs.h>

#ifndef __ASSEMBLY__

/// @brief Read and parse SecApp's configuration files. 
/// @param out_cfg 
/// @return Return SL_SUCCESS on success.
///         Return SL_EINVAL if <out_cfg> is invalid.
///         Return SL_EINVAL_CONFIGS on configuration errors, because they must be caused by invalid settings in the 
///         configuration file.
extern SL_STATUS loader_params_read(SAP_CONFIG* out_cfg);

#endif // __ASSEMBLY__

#endif // _CONFIG_MGR_H