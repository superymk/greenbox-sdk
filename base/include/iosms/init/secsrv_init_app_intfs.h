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

#ifndef INIT_APP_SP_INTFS_H
#define INIT_APP_SP_INTFS_H

#include <vk_syscalls.h>

#ifndef __ASSEMBLY__


/// @brief Configurations of trusted path devices
struct init_conf_nonsecure
{
    // Wall-clock related info
    TIMEVAL wall_clock_tv;
	TIMEZONE wall_clock_tz;
    uint64_t wall_clock_access_tsc;
};


#endif // __ASSEMBLY__
#endif // INIT_APP_SP_INTFS_H