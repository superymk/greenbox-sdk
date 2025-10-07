// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIB_INIT_OPS_H
#define LIB_INIT_OPS_H

#define LIB_INIT_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO

#ifndef __ASSEMBLY__

#define INIT_DEBUG(format, ...) \
	do{	\
		if((LIB_INIT_LOG >= LOG_DEBUG) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-INIT][Debug] "format, ## __VA_ARGS__); } }while(0)

#define INIT_INFO(format, ...) \
	do{	\
		if((LIB_INIT_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-INIT] "format, ## __VA_ARGS__); } }while(0)

#define INIT_ERR(format, ...) \
	do{	\
		if((LIB_INIT_LOG >= LOG_ERR) && (SAP_LOG != LOG_NONE)) {  \
		printf("[Lib-INIT][ERROR] "format, ## __VA_ARGS__); } }while(0)

extern SAP_STATUS secsrv_init_init(spid_t secsrv_init_spid);
extern void secsrv_init_fini(void);

/// @brief Update the priority of the given SecProcess.
/// @param target_sp_id 
/// @param new_priority 
/// @return 
extern SAP_STATUS secsrv_init_sp_priority_update(spid_t target_sp_id, sp_priority_t new_priority);

#endif // __ASSEMBLY__
#endif // LIB_INIT_OPS_H