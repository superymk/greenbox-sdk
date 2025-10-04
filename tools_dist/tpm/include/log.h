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
