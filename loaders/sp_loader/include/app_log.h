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

#ifndef APP_LOGS_H
#define APP_LOGS_H

// Include for sl_printf
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "app_log_file.h"

#ifdef _MSC_VER
// Microsoft Visual C++ does not support __restrict__ for C (though it does for C++ as __restrict)
#define RESTRICT
#else
// If not compiling with Microsoft Visual C++, assume __restrict__ is supported
#define RESTRICT __restrict__
#endif // _MSC_VER

// Undefine LOG_* as they may be defined in other libraries for the untrusted OS/Apps
#undef  LOG_NONE
#undef  LOG_EMERG
#undef  LOG_ALERT
#undef  LOG_CRIT
#undef  LOG_ERR
#undef  LOG_WARNING
#undef  LOG_NOTICE
#undef  LOG_INFO
#undef  LOG_PERF
#undef  LOG_DEBUG


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

#define APP_LOG				LOG_INFO
#define APP_COMM_LOG		LOG_INFO
#define HID_BRIDGE_LOG		LOG_INFO
#define GUI_BRIDGE_LOG		LOG_INFO
#define TPM_BRIDGE_LOG		LOG_INFO

#ifndef __ASSEMBLY__
static inline int sl_printf(const char * RESTRICT fmt, ...)
{
#ifdef XMHF_LOG_BUF_SIZE
	{
		// Print to XMHF
		uintptr_t a = 0x79786c;
		char buf[XMHF_LOG_BUF_SIZE];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, XMHF_LOG_BUF_SIZE, fmt, ap);
		va_end(ap);
		__asm__ __volatile__("vmcall" : "+a"(a) : "c"(7), "d"(0), "S"(0),
							 "D"((uintptr_t) buf));
	}
#endif /* XMHF_LOG_BUF_SIZE */
	{
		// Print to terminal
		int ret;
		va_list ap;
        va_start(ap, fmt);
        (void)app_log_file_vlog(fmt, ap);
		va_end(ap);
		va_start(ap, fmt);
		ret = vprintf(fmt, ap);
		va_end(ap);
		return ret;
	}
}

#define APP_DEBUG(format, ...) \
	do{	\
		if((APP_LOG >= LOG_DEBUG) && (APP_LOG != LOG_NONE)) {  \
		sl_printf("[App DEBUG] "format, ## __VA_ARGS__); } }while(0)

#define APP_INFO(format, ...) \
	do{	\
		if((APP_LOG >= LOG_INFO) && (APP_LOG != LOG_NONE)) {  \
		sl_printf("[App] "format, ## __VA_ARGS__); } }while(0)

#define APP_WARN(format, ...) \
	do{	\
		if((APP_LOG >= LOG_WARNING) && (APP_LOG != LOG_NONE)) {  \
		sl_printf("[App WARN] "format, ## __VA_ARGS__); } }while(0)


#define APP_ERR(format, ...) \
	do{	\
		if((APP_LOG >= LOG_ERR) && (APP_LOG != LOG_NONE)) {  \
		sl_printf("[App ERROR] "format, ## __VA_ARGS__); } }while(0)


#define APP_CHECK(x)        {		\
		do {																  \
			if(!(x)) {															\
				sl_printf("[App:Invalid Parameter][%s:%d]: %s\n", __FILE__, __LINE__, #x); \
				goto out;														 \
			}																	\
		} while(0); }


#endif //__ASSEMBLY__
#endif // APP_LOGS_H