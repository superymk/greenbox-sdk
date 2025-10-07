// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _APP_DDK_H
#define _APP_DDK_H

#include <os/os_headers.h>

#include <app_types.h>
#include <app_var.h>
#include <secloader_status.h>

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

// Include App-SecApp common definitions 
#include <secapp/app_sp_common_defs.h>

// AppLoader related headers
#include <ipc/app_comm_ops.h>
#include <ipc/sl_rpc/sl_rpc_protocol.h>
#include <ipc/sl_rpc/sl_rpc_ops.h>
#include <app_stdlib.h>
#include <app_utils.h>
#include <utils_mem.h>
#include <app_os_if.h>
#include <app_locks.h>
#include <app_pipe.h>
#include <app_sa_intfs.h>
#include <app_sb_intfs.h>
#include <sa_notify.h>
#include <app_loader.h>
#include <app_params_mem.h>
#include <app_sharedmem_mgr.h>
#include <sp_states.h>

#include <cJSON.h>

#endif // _APP_DDK_H