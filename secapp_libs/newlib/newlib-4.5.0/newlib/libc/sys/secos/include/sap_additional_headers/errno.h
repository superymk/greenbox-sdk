// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_ERRNO_H
#define SECAPP_ERRNO_H

#include <vk_errno.h>

#define EAGAIN      		    11  // Try again
#define E_SECOS_SYSCALL         1000


#ifndef __ASSEMBLY__

typedef ulong_t 				SAP_STATUS;

#endif // __ASSEMBLY__
#endif // SECAPP_ERRNO_H