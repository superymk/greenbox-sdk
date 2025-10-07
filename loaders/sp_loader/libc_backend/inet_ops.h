// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBC_BACKEND_INET_H
#define LIBC_BACKEND_INET_H

#include <libc_backend.h>

#ifndef __ASSEMBLY__

extern size_t do_CMD_INET_ADDR(struct libc_msg* msg);
extern size_t do_CMD_INET_NTOA(struct libc_msg* msg);
extern size_t do_CMD_INET_PTON(struct libc_msg* msg);
extern size_t do_CMD_INET_NTOP(struct libc_msg* msg);

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_INET_H