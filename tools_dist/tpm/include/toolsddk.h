// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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