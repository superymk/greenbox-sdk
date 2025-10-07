// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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