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