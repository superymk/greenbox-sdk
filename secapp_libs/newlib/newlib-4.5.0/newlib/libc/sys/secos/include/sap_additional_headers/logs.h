// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_LOGS_H
#define SECAPP_LOGS_H

#define LOG_NONE		0
#define LOG_EMERG 		1 //! system is unusable
#define LOG_ALERT 		2 //! action must be taken immediately
#define LOG_CRIT 		3 //! critical conditions
#define LOG_ERR 		4 //! error conditions
#define LOG_WARNING 	5 //! warning conditions
#define LOG_NOTICE 		6 //! normal but significant condition
#define LOG_INFO 		7 //! informational
#define LOG_PERF  		8
#define LOG_DEBUG 		9 //! debug-level messages


#define SAP_LOG 		LOG_INFO // LOG_DEBUG   LOG_INFO


#ifndef __ASSEMBLY__

#define SAP_DEBUG(format, ...) \
	do{	\
		if((SAP_LOG >= LOG_DEBUG) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess (id:%u)][Debug] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)
#define SAP_INFO(format, ...) \
	do{ \
        if((SAP_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess (id:%u)] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)
#define SAP_INFOs(format, ...) \
	do{ \
        if((SAP_LOG >= LOG_INFO) && (SAP_LOG != LOG_NONE)) {  \
		printf(format, ## __VA_ARGS__); } }while(0)
#define SAP_WARN(format, ...) \
	do{ \
        if((SAP_LOG >= LOG_WARNING) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess (id:%u)][WARN] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)
#define SAP_ERR(format, ...) \
	do{ \
        if((SAP_LOG >= LOG_ERR) && (SAP_LOG != LOG_NONE)) {  \
		printf("[SecProcess (id:%u)][ERR] "format, current_sp->sp_id, ## __VA_ARGS__); } }while(0)


#endif //__ASSEMBLY__
#endif // SECAPP_LOGS_H