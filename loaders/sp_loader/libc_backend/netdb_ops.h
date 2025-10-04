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

#ifndef LIBC_BACKEND_NETDB_H
#define LIBC_BACKEND_NETDB_H

#include <libc_backend.h>

#ifndef __ASSEMBLY__

extern size_t do_CMD_NETDB_ENDHOSTENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_ENDNETENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_ENDPROTOENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_ENDSERVENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETHOSTBYADDR(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETHOSTBYNAME(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETHOSTENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETNETBYADDR(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETNETBYNAME(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETNETENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETPROTOBYNAME(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETPROTOBYNUMBER(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETPROTOENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETSERVBYNAME(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETSERVBYPORT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETSERVENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_SETHOSTENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_SETNETENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_SETPROTOENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_SETSERVENT(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETADDRINFO(struct libc_msg* msg);
extern size_t do_CMD_NETDB_GETNAMEINFO(struct libc_msg* msg);

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_NETDB_H