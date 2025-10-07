// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECOS_SECPROCESS_LOADER_STATUS_H
#define SECOS_SECPROCESS_LOADER_STATUS_H

#define SL_SUCCESS		 	0
#define SL_EIO              5   // I/O error
#define SL_ENOMEM           12  // Out of memory
#define SL_EINVAL      		22  // Invalid argument
#define SL_ETIMEOUT      	34  // Operation Timeout
// #define SL_SECOS_TIMEOUT    100
#define SL_EINVAL_CONFIGS   122

#define SL_EINTERNAL		999

#ifndef __ASSEMBLY__

#define SL_FAIL(status)		(unlikely(status != SL_SUCCESS))
#define SL_PASS(status)		(likely(status == SL_SUCCESS))

typedef unsigned long SL_STATUS;

#endif // __ASSEMBLY__
#endif // SECOS_SECPROCESS_LOADER_STATUS_H