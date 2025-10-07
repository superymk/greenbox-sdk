// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef OS_LINUX
#define OS_LINUX

#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include "compilers.h"

#ifndef __ASSEMBLY__

#ifdef XMHF_LOG_BUF_SIZE

static inline void PRINT(const char *fmt, ...)
{
	uintptr_t a = 0x79786c;
	char buf[XMHF_LOG_BUF_SIZE];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, XMHF_LOG_BUF_SIZE, fmt, ap);
	va_end(ap);
	__asm__ __volatile__("vmcall" : "+a"(a) : "c"(5), "d"(0), "S"(0),
						 "D"((uintptr_t) buf));
}

#define PRINT_ERR(format, ...) \
	do{ \
		PRINT(format, ## __VA_ARGS__); \
		printk(KERN_ALERT format, ## __VA_ARGS__); \
	} while(0)

#else /* !XMHF_LOG_BUF_SIZE */

#define PRINT(format, ...) \
	do{	\
		printk(KERN_INFO format, ## __VA_ARGS__); } while(0)

#define PRINT_ERR(format, ...) \
	do{	\
		printk(KERN_ALERT format, ## __VA_ARGS__); } while(0)

#endif /* XMHF_LOG_BUF_SIZE */

// Force output to untrusted OS (not mHV)
#define PRINT_ERR_FORCE_OS(format, ...) \
	do{	\
		printk(KERN_ALERT format, ## __VA_ARGS__); } while(0)

typedef unsigned long ulong_t;

typedef spinlock_t lock_t;


#endif // __ASSEMBLY__
#endif // OS_LINUX
