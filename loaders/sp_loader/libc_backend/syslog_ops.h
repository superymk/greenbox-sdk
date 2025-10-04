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

#ifndef LIBC_BACKEND_SYSLOG_H
#define LIBC_BACKEND_SYSLOG_H

#include <libc_backend.h>

#ifndef __ASSEMBLY__

extern size_t do_CMD_SYSLOG_CLOSE(struct libc_msg* msg);
extern size_t do_CMD_SYSLOG_OPEN(struct libc_msg* msg);
extern size_t do_CMD_SYSLOG_SETLOGMASK(struct libc_msg* msg);
extern size_t do_CMD_SYSLOG_SYSLOG(struct libc_msg* msg);


#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_SYSLOG_H