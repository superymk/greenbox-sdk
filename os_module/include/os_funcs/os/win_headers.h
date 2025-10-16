// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef OS_WINDOWS
#define OS_WINDOWS

#include <ntddk.h>
#include <wdf.h>
#include <stdarg.h>
#include <stdio.h>

#include "compilers.h"

// Configuration file that defines XMHF_LOG_BUF_SIZE
#include "win_config_xmhf_log.h"

// Config - Force using legacy UXMHF64.
#undef GREENBOX_USE_UXMHF64_V1
// #define GREENBOX_USE_UXMHF64_V1 1

#ifdef _AMD64_
#undef __i386__
#define __amd64__
#endif

#ifndef __ASSEMBLY__

#pragma warning(disable: 4127) // Disable warning C4127: conditional expression is constant
#pragma warning(disable: 4201) // Disable warning C4201: nonstandard extension used: nameless struct/union

// Force output to untrusted OS (not mHV)
#define PRINT_ERR_FORCE_OS(fmt, ...) \
  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt, __VA_ARGS__)

#ifdef XMHF_LOG_BUF_SIZE

extern uintptr_t Intel_VMCALL_ret4(uintptr_t fn, uintptr_t* ret_cx, uintptr_t* ret_dx, uintptr_t* ret_si, uintptr_t* ret_di);

static inline void PRINT(const char *fmt, ...)
{
    char buf[XMHF_LOG_BUF_SIZE];
	uintptr_t a = 0x79786c, c=5, d=0, S=0, D=(uintptr_t)buf;
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buf, XMHF_LOG_BUF_SIZE, fmt, ap);
	va_end(ap);
    Intel_VMCALL_ret4(a, &c, &d, &S, &D);
}

#define PRINT_ERR(fmt, ...) \
  PRINT(fmt, __VA_ARGS__)

#else /* !XMHF_LOG_BUF_SIZE */

#define PRINT(fmt, ...) \
  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt, __VA_ARGS__)

#define PRINT_ERR(fmt, ...) \
  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt, __VA_ARGS__)

#endif /* XMHF_LOG_BUF_SIZE */



#define SA_TAG (ULONG)'SECA'

// Additional typedefs required by windows
typedef unsigned char		bool;
typedef ULONG_PTR ulong_t;

#define true				1
#define false				0

#define likely(x)		(x)
#define unlikely(x)		(x)

#define __attribute__(x) /* blank - should simply ignore thanks to C preprocessor */

// error code
#define ENOMEM          12      // Out of memory
#define EINVAL          22      // Invalid argument

typedef struct
{
	KSPIN_LOCK lock;
	KIRQL irql;
} lock_t;

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
    PKINTERRUPT InterruptObject;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _FILE_CONTEXT{
    //
    // Lock to rundown threads that are dispatching I/Os on a file handle 
    // while the cleanup for that handle is in progress.
    //
    IO_REMOVE_LOCK  FileRundownLock;
} FILE_CONTEXT, *PFILE_CONTEXT;

#define IOCTL_SECAGENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif // __ASSEMBLY__
#endif // OS_WINDOWS

