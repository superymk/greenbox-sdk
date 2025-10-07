// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBC_BACKEND_IOCTL_H
#define LIBC_BACKEND_IOCTL_H

#include <libc_backend.h>

#ifndef __ASSEMBLY__

extern size_t do_CMD_IOCTL_ARG1_RET(struct libc_msg* msg);


#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_IOCTL_H