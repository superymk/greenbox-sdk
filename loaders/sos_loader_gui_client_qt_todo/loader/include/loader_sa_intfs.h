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