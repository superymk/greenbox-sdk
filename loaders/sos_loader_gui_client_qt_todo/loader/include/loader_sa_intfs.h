// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LOADER_SECAGENT_INTFS_H
#define LOADER_SECAGENT_INTFS_H

// #define _GNU_SOURCE
#include <app_types.h>
// #include <os_headers.h>
#include <secagent_intfs.h>
#include <os_errno.h>

#ifndef __ASSEMBLY__

/// @brief Share data with any NS domain's process. The current will create map <selector, data> in SecAgent. Any
/// proceseses can get the data with the key <selector>.
/// @param selector 
/// @param data 
/// @return 
extern OSSTATUS secagent_nsdom_share_data_to_any_ns_process_set(uint64_t selector, uint64_t data);

/// @brief Get shared data with any NS domain's process. The current will create map <selector, data> in SecAgent. Any
/// proceseses can get the data with the key <selector>.
/// @param selector 
/// @param data 
/// @return 
extern OSSTATUS secagent_nsdom_share_data_to_any_ns_process_get(uint64_t selector, uint64_t* data);

#endif // __ASSEMBLY__
#endif // LOADER_SECAGENT_INTFS_H