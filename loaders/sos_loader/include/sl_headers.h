// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOSLD_COMMON_HEADERS_H
#define SOSLD_COMMON_HEADERS_H

// Include Apps public headers
#include <os/os_headers.h>
#include <app_types.h>
#include <app_var.h>
#include <app_log.h>
#include <app_os_if.h>
#include <app_locks.h>
#include <app_utils.h>

// Include SecOS public headers
#include <vk_configs.h>
#include <sos_common_defs.h>
#include <vk_common_intfs.h>
#include <vk_mm_defs.h>
#include <vk_app_intfs.h>

// Include SecAgent public headers
#include <secagent.h>

// Include headers for itself
#include <sl_status.h>
#include <sl_states.h>
#include <sl_malloc.h>
#include <utils.h>
#include <sl_os_intfs.h>
#include <sl_sb_intfs.h>
#include <sl_pipe.h>
// #include <sl_os.h>
#include <sl_secos.h>
#include <sl_secos_tasks_after_unload.h>
#include <lib_sl.h>
#include <sl_cfg_reader.h>
#include <sosl_log.h>

#define SL_LOG			LOG_INFO

#ifndef __ASSEMBLY__

#define SL_DEBUG(format, ...) \
	do{	\
		if((SL_LOG >= LOG_DEBUG) && (SL_LOG != LOG_NONE)) {  \
		sl_printf("[SecOS Loader][Debug] "format, ## __VA_ARGS__); } }while(0)
		
#define SL_INFO(format, ...) \
	do{	\
		if((SL_LOG >= LOG_INFO) && (SL_LOG != LOG_NONE)) {  \
		sl_printf("[SecOS Loader] "format, ## __VA_ARGS__); } }while(0)

#define SL_WARN(format, ...) \
	do{	\
		if((SL_LOG >= LOG_WARNING) && (SL_LOG != LOG_NONE)) {  \
		sl_printf("[SecOS Loader][WARN] "format, ## __VA_ARGS__); } }while(0)

#define SL_ERR(format, ...) \
	do{	\
		if((SL_LOG >= LOG_ERR) && (SL_LOG != LOG_NONE)) {  \
		sl_printf("[SecOS Loader][ERR] "format, ## __VA_ARGS__); } }while(0)

#endif // __ASSEMBLY__
#endif // SOSLD_COMMON_HEADERS_H
