// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef OS_LOGS_H
#define OS_LOGS_H

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

#define DRV_LOG		LOG_INFO
#define SACOMM_LOG	LOG_INFO // LOG_DEBUG //  LOG_NONE
#define SOS_MM_LOG	LOG_INFO

#ifndef __ASSEMBLY__

#define DRV_DEBUG(format, ...) \
	do{	\
		if((DRV_LOG >= LOG_DEBUG) && (DRV_LOG != LOG_NONE)) {  \
		PRINT("[CPU:%u][SecAgent-Debug] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define DRV_INFO(format, ...) \
	do{	\
		if((DRV_LOG >= LOG_INFO) && (DRV_LOG != LOG_NONE)) {  \
		PRINT("[CPU:%u][SecAgent] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)
#define DRV_INFOs(format, ...) \
	do{	\
		if((DRV_LOG >= LOG_INFO) && (DRV_LOG != LOG_NONE)) {  \
		PRINT(format, ## __VA_ARGS__); } }while(0)

#define DRV_WARN(format, ...) \
	do{	\
		if((DRV_LOG >= LOG_WARNING) && (DRV_LOG != LOG_NONE)) {  \
		PRINT_ERR("[CPU:%u][SecAgent-WARN] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define DRV_ERR(format, ...) \
	do{	\
		if((DRV_LOG >= LOG_ERR) && (DRV_LOG != LOG_NONE)) {  \
		PRINT_ERR("[CPU:%u][SecAgent-ERR] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#endif //__ASSEMBLY__
#endif // OS_LOGS_H
