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

#ifndef SECAGENT_H
#define SECAGENT_H

#ifdef __linux__
	#include <os_funcs/os/linux_headers.h>
#elif defined (_WIN32) || defined (_WIN64)
	#include <os_funcs/os/win_headers.h>
#endif

// Fundamental Utility Definitions
#include <os_types.h>
#include <os_var.h>
#include <os_logs.h>
#include <public/os_errno.h>
#include <utils.h>
#include <stl_dlist.h>
#include <perf.h>
#include <debug.h>

// SecBase headers
#include <secbase.h>
#include <secbase_errno.h>
#include <secbase_hypercalls.h>
#include <secbase_ns_intfs.h>

// SecOS headers for SecAgent
#include <vk_configs.h>
#include <sos_common_defs.h>
#include <secapp_defs.h>
#include <vk_mm_defs.h>
#include <vk_errno.h>
#include <vk_os_module.h>
#include <vk_comm_os.h>
#include <vk_common_intfs.h>
#include <vk_app_intfs.h>

#include <x86_mm.h>

// App headers for SecAgent
#include <public/secagent.h>

// APIs
#include <arch/x86/headers.h>

#include <os_funcs.h>
#include <apic.h>
#include <mm.h>
#include <intr.h>
#include <sa_process.h>
#include <untrusted_loader_info.h>
#include <untrusted_loader_utils.h>
#include <app_notif_mgr.h>
#include <sos_mm_mgr.h>
#include <comm_sos.h>
#include <comm_sos_handler.h>
#include <ioctl_handler.h>
#include <secagent_ops.h>
#include <sos_sp_params.h>
#include <sos.h>

#endif // SECAGENT_H