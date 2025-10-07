// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef TPMTool_LOGS_H
#define TPMTool_LOGS_H

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

#define TOOL_LOG		LOG_INFO

#ifndef __ASSEMBLY__

#define TOOL_DEBUG(format, ...) \
	do{	\
		if((TOOL_LOG >= LOG_DEBUG) && (TOOL_LOG != LOG_NONE)) {  \
		printf("[TPMTool-Debug] "format, ## __VA_ARGS__); } }while(0)
#define TOOL_PERF(format, ...) \
	do{ \
        if((TOOL_LOG >= LOG_PERF) && (TOOL_LOG != LOG_NONE)) {  \
		printf("[TPMTool-PERF] "format, ## __VA_ARGS__); } }while(0)
#define TOOL_INFO(format, ...) \
	do{ \
        if((TOOL_LOG >= LOG_INFO) && (TOOL_LOG != LOG_NONE)) {  \
		printf("[TPMTool] "format, ## __VA_ARGS__); } }while(0)
#define TOOL_INFO_S(format, ...) \
	do{ \
        if((TOOL_LOG >= LOG_INFO) && (TOOL_LOG != LOG_NONE)) {  \
		printf(format, ## __VA_ARGS__); } }while(0)
#define TOOL_WARN(format, ...) \
	do{ \
        if((TOOL_LOG >= LOG_WARNING) && (TOOL_LOG != LOG_NONE)) {  \
		printf("[TPMTool-WARN] "format, ## __VA_ARGS__); } }while(0)
#define TOOL_ERR(format, ...) \
	do{ \
        if((TOOL_LOG >= LOG_ERR) && (TOOL_LOG != LOG_NONE)) {  \
		printf("[TPMTool-ERR] "format, ## __VA_ARGS__); } }while(0)


#endif //__ASSEMBLY__
#endif // TPMTool_LOGS_H
