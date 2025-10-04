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

#ifndef _TOOLS_DDK_H
#define _TOOLS_DDK_H

#include <os/os_headers.h>

#include <app_types.h>
#include <app_var.h>

/* The public folder contains headers for apps, pals, OSes, etc. */
#include <vk_mm_defs.h>
#include <vk_configs.h>
#include <app_log.h>

#include <app_sp_configs.h>

// Include SecOS public headers (for app)
#include <vk_configs.h>
#include <sos_common_defs.h>
#include <vk_common_intfs.h>
#include <vk_app_intfs.h>
#include <vk_mm_defs.h>
#include <sos_sp_intfs.h>
#include <secapp_defs.h>

// Include SecAgent public headers (for app)
#include <secagent.h>

// Include SecOS loader public headers
#include <secbase_errno.h>
#include <secbase.h>
#include <secbase_hypercalls.h>



#endif // _TOOLS_DDK_H