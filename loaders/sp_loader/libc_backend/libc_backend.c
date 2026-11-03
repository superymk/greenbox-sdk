// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <libc_backend.h>
#include <app_log.h>
#include <app_stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "_libc_cmds.h"
#include "_info.h"

#include "fd_ops.h"
#include "inet_ops.h"
#include "netdb_ops.h"
#include "socket_ops.h"
#include "time_ops.h"
#include "syslog_ops.h"
#include "stat_ops.h"
#include "ioctl_ops.h"

#define HANDLE(hc)                  \
    case hc:                        \
        msg_data_sz = do_##hc(msg); \
        break

void libc_clear_errno(void)
{
    errno = 0;
    barrier();
}

size_t handle_libc_function(struct libc_msg *msg, bool *out_valid_msg)
{
    size_t msg_data_sz = 0;
    bool valid_msg = true;

    // [Debug] Print the call to the libc-backend.
    _libc_backend_dbg_print_cmd(msg->cmd);

    switch (msg->cmd)
    {
        // File
        HANDLE(CMD_FILE_OPEN);
        HANDLE(CMD_FILE_READ);
        HANDLE(CMD_FILE_WRITE);
        HANDLE(CMD_FILE_CLOSE);
        HANDLE(CMD_FILE_FSTAT);
        HANDLE(CMD_FILE_STAT);
        HANDLE(CMD_FILE_ISATTY);
        HANDLE(CMD_FILE_SELECT);
        HANDLE(CMD_FILE_FCNTL);
        HANDLE(CMD_FILE_LSEEK);
        HANDLE(CMD_FILE_POLL);
        HANDLE(CMD_FILE_RENAME);
        HANDLE(CMD_FILE_ACCESS);

        // Dir
        HANDLE(CMD_FILEDIR_MKDIR);
        HANDLE(CMD_FILEDIR_OPENDIR);
        HANDLE(CMD_FILEDIR_READDIR);
        HANDLE(CMD_FILEDIR_CLOSEDIR);

        // Stat
        HANDLE(CMD_STAT_CHMOD);
        HANDLE(CMD_STAT_FCHMOD);

        // IOCTL
        HANDLE(CMD_IOCTL_ARG1_RET);

        // Socket
        HANDLE(CMD_SOCK_ACCEPT);
        HANDLE(CMD_SOCK_BIND);
        HANDLE(CMD_SOCK_CONNECT);
        HANDLE(CMD_SOCK_GETPEERNAME);
        HANDLE(CMD_SOCK_GETSOCKNAME);
        HANDLE(CMD_SOCK_RECV);
        HANDLE(CMD_SOCK_SEND);
        HANDLE(CMD_SOCK_GETSOCKOPT);
        HANDLE(CMD_SOCK_SETSOCKOPT);
        HANDLE(CMD_SOCK_SOCKET);
        HANDLE(CMD_SOCK_SOCKETPAIR);
        HANDLE(CMD_SOCK_LISTEN);
        HANDLE(CMD_SOCK_SHUTDOWN);
        HANDLE(CMD_SOCK_SOCKATMARK);
        HANDLE(CMD_SOCK_RECVFROM);

        // Inet
        HANDLE(CMD_INET_ADDR);
        HANDLE(CMD_INET_NTOA);
        HANDLE(CMD_INET_PTON);
        HANDLE(CMD_INET_NTOP);

        // NETDB
        HANDLE(CMD_NETDB_ENDHOSTENT);
        HANDLE(CMD_NETDB_ENDPROTOENT);
        HANDLE(CMD_NETDB_ENDSERVENT);
        HANDLE(CMD_NETDB_GETHOSTBYADDR);
        HANDLE(CMD_NETDB_GETHOSTBYNAME);
        HANDLE(CMD_NETDB_GETPROTOBYNAME);
        HANDLE(CMD_NETDB_GETPROTOBYNUMBER);
        HANDLE(CMD_NETDB_GETPROTOENT);
        HANDLE(CMD_NETDB_GETSERVBYNAME);
        HANDLE(CMD_NETDB_GETSERVBYPORT);
        HANDLE(CMD_NETDB_GETSERVENT);
        HANDLE(CMD_NETDB_SETHOSTENT);
        HANDLE(CMD_NETDB_SETPROTOENT);
        HANDLE(CMD_NETDB_SETSERVENT);
        HANDLE(CMD_NETDB_GETADDRINFO);
        HANDLE(CMD_NETDB_GETNAMEINFO);

        // Time
        HANDLE(CMD_TIME_GETTIMEOFDAY);
        HANDLE(CMD_TIME_TIMES);

        // Syslog
        HANDLE(CMD_SYSLOG_CLOSE);

    default:
        ERR("Invalid Command!\n");
        valid_msg = false;
        break;
    }

    *out_valid_msg = valid_msg;

    return msg_data_sz;
}

/********* Debug Functions *********/
void _libc_backend_dbg_print_cmd(uint32_t cmd)
{
    if (
        (cmd != CMD_FILE_READ) && 
        (cmd != CMD_FILE_SELECT) && 
        (cmd != CMD_FILE_POLL)
    )
    {
        APP_DEBUG("[Superymk] <handle_libc_function> msg->cmd:%u\n", (cmd));
    }
}

void _libc_backend_dbg_print_mem_as_string(char* prefix, void* buf, size_t len)
{
    uint8_t buf2[20] = {0};

    memcpy(buf2, buf, min(20, len));
    APP_DEBUG("[%s] %s\n", prefix, (char*)buf2);
}
