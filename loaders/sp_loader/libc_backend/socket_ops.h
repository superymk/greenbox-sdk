// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBC_BACKEND_SOCKET_H
#define LIBC_BACKEND_SOCKET_H

#include <libc_backend.h>

#ifndef __ASSEMBLY__

extern size_t do_CMD_SOCK_ACCEPT(struct libc_msg* msg);
extern size_t do_CMD_SOCK_BIND(struct libc_msg* msg);
extern size_t do_CMD_SOCK_CONNECT(struct libc_msg* msg);
extern size_t do_CMD_SOCK_GETPEERNAME(struct libc_msg* msg);
extern size_t do_CMD_SOCK_GETSOCKNAME(struct libc_msg* msg);
extern size_t do_CMD_SOCK_RECV(struct libc_msg* msg);
extern size_t do_CMD_SOCK_SEND(struct libc_msg* msg);
extern size_t do_CMD_SOCK_GETSOCKOPT(struct libc_msg* msg);
extern size_t do_CMD_SOCK_SETSOCKOPT(struct libc_msg* msg);
extern size_t do_CMD_SOCK_SOCKET(struct libc_msg* msg);
extern size_t do_CMD_SOCK_SOCKETPAIR(struct libc_msg* msg);
extern size_t do_CMD_SOCK_LISTEN(struct libc_msg* msg);
extern size_t do_CMD_SOCK_SHUTDOWN(struct libc_msg* msg);
extern size_t do_CMD_SOCK_SOCKATMARK(struct libc_msg* msg);
extern size_t do_CMD_SOCK_RECVFROM(struct libc_msg* msg);

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_SOCKET_H