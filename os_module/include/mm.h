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

#ifndef SECAGENT_MM_H
#define SECAGENT_MM_H
#include <os_types.h>


#ifndef __ASSEMBLY__

// extern OSSTATUS do_CMD_APP_CACHE_FLUSH_RANGE(struct tz_cmd_packet *cmd_packet);

/// @brief Allocate and lock a memory region of <num_pages> for the S domain. The memory region is not zeroed to improve  
/// SecAgent responsiveness because the region could be very large.
/// @param cmd_packet Arg1: <num_pages> of uint32_t. Number of 4K pages to be allocated and locked.
/// @return  
extern OSSTATUS do_CMD_ALLOC_AND_LOCK_PAGES_FOR_SDOM(struct tz_cmd_packet *cmd_packet);

/// @brief Free and unlock the memory region allocated for the S domain.
/// @param cmd_packet Arg1: <uvaddr> of uvaddr_t. The user-mode vaddr of the memory region.
/// @return 
extern OSSTATUS do_CMD_FREE_AND_UNLOCK_PAGES_FOR_SDOM(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_MM_H