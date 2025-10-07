// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBC_BACKEND_FD_H
#define LIBC_BACKEND_FD_H

#include <libc_backend.h>

#ifndef __ASSEMBLY__

extern size_t do_CMD_FILE_OPEN(struct libc_msg* msg);
extern size_t do_CMD_FILE_CLOSE(struct libc_msg* msg);
extern size_t do_CMD_FILE_READ(struct libc_msg* msg);
extern size_t do_CMD_FILE_WRITE(struct libc_msg* msg);
extern size_t do_CMD_FILE_FSTAT(struct libc_msg* msg);
extern size_t do_CMD_FILE_STAT(struct libc_msg* msg);
extern size_t do_CMD_FILE_ISATTY(struct libc_msg* msg);
extern size_t do_CMD_FILE_SELECT(struct libc_msg* msg);
extern size_t do_CMD_FILE_FCNTL(struct libc_msg* msg);
extern size_t do_CMD_FILE_LSEEK(struct libc_msg* msg);
extern size_t do_CMD_FILE_POLL(struct libc_msg* msg);
extern size_t do_CMD_FILE_RENAME(struct libc_msg* msg);
extern size_t do_CMD_FILE_ACCESS(struct libc_msg* msg);

extern size_t do_CMD_FILEDIR_MKDIR(struct libc_msg* msg);
extern size_t do_CMD_FILEDIR_OPENDIR(struct libc_msg* msg);
extern size_t do_CMD_FILEDIR_READDIR(struct libc_msg* msg);
extern size_t do_CMD_FILEDIR_CLOSEDIR(struct libc_msg* msg);

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_FD_H