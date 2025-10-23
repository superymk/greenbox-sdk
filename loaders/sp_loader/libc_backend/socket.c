// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>

#include <app_log.h>
#include <app_stdlib.h>
#include <ipc/varbuf.h>
#include <vk_mm_defs.h>

#include "_info.h"
#include "socket_ops.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"

static bool _deserialize_sockaddr(varbuf_t *varbuf, struct sockaddr_storage *sock)
{
    bool status = false;
    uint32_t datasz = 0;

    // Sanity check
    if (!sock)
        return false;

    // Get size
    status = varbuf_tryread_datasz(varbuf, &datasz);
    if (!status)
        return false;

    // sock
    status = varbuf_read_fixdata(varbuf, sock, datasz);
    if (!status)
        return false;

    // Translate newlib's socket input constants to OS-specific posix definitions
    sock->ss_family = (sa_family_t)newlib_to_os_socket_addr_family(sock->ss_family);

    return true;
}

static bool _serialize_sockaddr(varbuf_t *varbuf, struct sockaddr_storage *sock, socklen_t addrlen)
{
    bool status = false;

    // Sanity check
    if (!sock || !addrlen)
        return false;

    if (addrlen > sizeof(struct sockaddr_storage))
        return false;

    // Translate OS-specific posix definitions to newlib's constants 
    sock->ss_family = (sa_family_t)os_to_newlib_socket_addr_family(sock->ss_family);

    // sock
    status = varbuf_write_data(varbuf, sock, addrlen);
    if (!status)
        return false;

    return true;
}

static bool _serialize_socklen_t(varbuf_t *varbuf, socklen_t addrlen)
{
    uint64_t addrlen_temp = (uint64_t)addrlen;

    if (!varbuf_write_data(varbuf, &addrlen_temp, sizeof(uint64_t))) // serialize socklen_t as uint64_t
    {
        return false;
    }

    // On success
    return true;
}

static bool _deserialize_socklen_t(varbuf_t *varbuf, socklen_t *out_addrlen)
{
    uint64_t addrlen_temp = 0;

    if (!varbuf_read_fixdata(varbuf, &addrlen_temp, sizeof(uint64_t)))
    {
        return false;
    }

    if (out_addrlen)
        *out_addrlen = (socklen_t)addrlen_temp;

    // On success
    return true;
}

size_t do_CMD_SOCK_ACCEPT(struct libc_msg *msg)
{
    // Origin: int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    int sockfd = 0;
    struct sockaddr_storage addr;
    socklen_t addrlen;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_ACCEPT Request Failed: <addr>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)accept(sockfd, (struct sockaddr *)&addr, &addrlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_ACCEPT Response Failed\n");

    if (!_serialize_sockaddr(varbuf, &addr, addrlen))
    {
        ERR("CMD_SOCK_ACCEPT Response Failed: <addr>\n");
    }

    if (!_serialize_socklen_t(varbuf, addrlen))
    {
        ERR("CMD_SOCK_ACCEPT Response Failed: <addrlen>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_BIND(struct libc_msg *msg)
{
    // Origin: int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    int sockfd = 0;
    struct sockaddr_storage addr;
    socklen_t addrlen;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_BIND Request Failed: <sockfd>\n");
        goto out;
    }

    if (!_deserialize_sockaddr(varbuf, &addr))
    {
        ERR("CMD_SOCK_BIND Request Failed: <addr>\n");
        goto out;
    }

    if (!_deserialize_socklen_t(varbuf, &addrlen))
    {
        ERR("CMD_SOCK_BIND Request Failed: <addrlen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)bind(sockfd, (struct sockaddr *)&addr, addrlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_BIND Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_CONNECT(struct libc_msg *msg)
{
    // Origin: int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    bool ret = false;

    int sockfd = 0;
    struct sockaddr_storage addr;
    socklen_t addrlen;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_CONNECT Request Failed: <sockfd>\n");
        goto out;
    }

    ret = _deserialize_sockaddr(varbuf, &addr);
    if (!ret)
    {
        ERR("CMD_SOCK_CONNECT Request Failed: <addr>\n");
        goto out;
    }

    if (!_deserialize_socklen_t(varbuf, &addrlen))
    {
        ERR("CMD_SOCK_CONNECT Request Failed: <addrlen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)connect(sockfd, (struct sockaddr *)&addr, addrlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_CONNECT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_GETPEERNAME(struct libc_msg *msg)
{
    // Origin: int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    int sockfd = 0;
    struct sockaddr_storage addr;
    socklen_t addrlen;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_GETPEERNAME Request Failed: <sockfd>\n");
        goto out;
    }

    if (!_deserialize_socklen_t(varbuf, &addrlen))
    {
        ERR("CMD_SOCK_GETPEERNAME Request Failed: <addrlen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)getpeername(sockfd, (struct sockaddr *)&addr, &addrlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_GETPEERNAME Response Failed\n");

    if (!_serialize_sockaddr(varbuf, &addr, addrlen))
        ERR("CMD_SOCK_GETPEERNAME Response Failed: <addr>\n");

    if (!_serialize_socklen_t(varbuf, addrlen))
        ERR("CMD_SOCK_GETPEERNAME Response Failed: <addrlen>\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_GETSOCKNAME(struct libc_msg *msg)
{
    // Origin: int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    int sockfd = 0;
    struct sockaddr_storage addr;
    socklen_t addrlen = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_GETSOCKNAME Request Failed: <sockfd>\n");
        goto out;
    }

    if (!_deserialize_socklen_t(varbuf, &addrlen))
    {
        ERR("CMD_SOCK_GETSOCKNAME Request Failed: <addrlen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)getsockname(sockfd, (struct sockaddr *)&addr, &addrlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);

    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <result>\n");

    if (!_serialize_sockaddr(varbuf, &addr, addrlen))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <addr>\n");

    if (!_serialize_socklen_t(varbuf, addrlen))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <addrlen>\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_RECV(struct libc_msg *msg)
{
    // Origin: ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    int sockfd = 0;
    size_t len = 0;
    int flags = 0;
    char *buf = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_RECV Request Failed: <sockfd>\n");
        goto out;
    }

    if (!varbuf_deserialize_size_t(varbuf, &len))
    {
        ERR("CMD_SOCK_RECV Request Failed: <len>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &flags, sizeof(int)))
    {
        ERR("CMD_SOCK_RECV Request Failed: <flags>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    flags = newlib_to_os_socket_send_recv_flags(flags);

    buf = malloc(len);
    if (!buf)
    {
        ERR("CMD_SOCK_RECV Request Failed: no mem for <buf>\n");
        goto out;
    }

    // Call the function
    libc_clear_errno();
    result = (uint64_t)recv(sockfd, buf, len, flags);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_RECV Response Failed\n");

    if (result != -1)
    {
        if (!varbuf_write_data(varbuf, buf, (uint32_t)result))
            ERR("CMD_SOCK_RECV Response Failed: <buf> buf:0x%lX, len:%u\n", (ulong_t)buf, (uint32_t)len);
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    if (buf)
        free(buf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SEND(struct libc_msg *msg)
{
    // Origin: ssize_t send(int sockfd, const void *buf, size_t len, int flags)
    int sockfd = 0;
    size_t len = 0;
    int flags = 0;
    void *buf = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_SEND Request Failed: <sockfd>\n");
        goto out;
    }

    if (!varbuf_deserialize_size_t(varbuf, &len))
    {
        ERR("CMD_SOCK_SEND Request Failed: <len>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &flags, sizeof(int)))
    {
        ERR("CMD_SOCK_SEND Request Failed: <flags>\n");
        goto out;
    }

    buf = varbuf_read_vardata(varbuf, NULL);
    if (!buf)
    {
        ERR("CMD_SOCK_SEND Request Failed: <buf>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    flags = newlib_to_os_socket_send_recv_flags(flags);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)send(sockfd, buf, len, flags);

out:
    if (buf)
        free(buf);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SEND Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_GETSOCKOPT(struct libc_msg *msg)
{
    // Origin: int     getsockopt(int sockfd, int level, int optname,
    //               void *restrict optval, socklen_t *restrict optlen)
    int sockfd = 0;
    int level = 0;
    int optname = 0;
    socklen_t optlen = 0;
    char *buf = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_GETSOCKOPT Request Failed: <sockfd>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &level, sizeof(int)))
    {
        ERR("CMD_SOCK_GETSOCKOPT Request Failed: <level>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &optname, sizeof(int)))
    {
        ERR("CMD_SOCK_GETSOCKOPT Request Failed: <optname>\n");
        goto out;
    }

    if (!_deserialize_socklen_t(varbuf, &optlen))
    {
        ERR("CMD_SOCK_GETSOCKOPT Request Failed: <optlen>\n");
        goto out;
    }

    // Translate newlib's socket input constants to OS-specific posix definitions
    level = newlib_to_os_socket_option_level(level);
    optname = newlib_to_os_socket_option_flags(optname);

    buf = (char *)malloc((uint32_t)optlen);
    if (!buf)
    {
        ERR("CMD_SOCK_GETSOCKOPT Request Failed: no memory for <buf>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)getsockopt(sockfd, level, optname, buf, &optlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_GETSOCKOPT Response Failed\n");

    if (!varbuf_write_data(varbuf, buf, optlen))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <buf>\n");

    if (!_serialize_socklen_t(varbuf, optlen))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <optlen>\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    if (buf)
        free(buf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SETSOCKOPT(struct libc_msg *msg)
{
    // Origin: int     setsockopt(int sockfd, int level, int optname,
    //               const void *optval, socklen_t optlen)
    int sockfd = 0;
    int level = 0;
    int optname = 0;
    socklen_t optlen = 0;
    void *buf = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_SETSOCKOPT Request Failed: <file>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &level, sizeof(int)))
    {
        ERR("CMD_SOCK_SETSOCKOPT Request Failed: <level>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &optname, sizeof(int)))
    {
        ERR("CMD_SOCK_SETSOCKOPT Request Failed: <optname>\n");
        goto out;
    }

    if (!_deserialize_socklen_t(varbuf, &optlen))
    {
        ERR("CMD_SOCK_SETSOCKOPT Request Failed: <optlen>\n");
        goto out;
    }

    if (optlen)
    {
        buf = varbuf_read_vardata(varbuf, NULL);
        if (!buf)
        {
            ERR("CMD_SOCK_SETSOCKOPT Request Failed: <optval>\n");
            goto out;
        }
    }
    else
    {
        if (!varbuf_read_nil(varbuf))
        {
            ERR("CMD_SOCK_SETSOCKOPT Request Failed: <optval> as NULL\n");
            goto out;
        }

        buf = NULL;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    level = newlib_to_os_socket_option_level(level);
    optname = newlib_to_os_socket_option_flags(optname);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)setsockopt(sockfd, level, optname, buf, optlen);

out:
    if (buf)
        free(buf);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SETSOCKOPT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SOCKET(struct libc_msg *msg)
{
    // Origin: int socket(int domain, int type, int protocol)
    int domain = 0;
    int type = 0;
    int protocol = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &domain, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKET Request Failed: <domain>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &type, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKET Request Failed: <type>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &protocol, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKET Request Failed: <protocol>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    domain = newlib_to_os_socket_addr_family(domain);
    type = newlib_to_os_socket_type(type);
    protocol = newlib_to_os_socket_protocol(protocol);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)socket(domain, type, protocol);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SOCKET Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SOCKETPAIR(struct libc_msg *msg)
{
    // Origin: int socketpair(int domain, int type, int protocol, int sv[2])
    int domain = 0;
    int type = 0;
    int protocol = 0;
    int sv[2] = {0};

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &domain, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKETPAIR Request Failed: <domain>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &type, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKETPAIR Request Failed: <type>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &protocol, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKETPAIR Request Failed: <protocol>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &sv[0], sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKETPAIR Request Failed: <sv[0]>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &sv[1], sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKETPAIR Request Failed: <sv[1]>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    domain = newlib_to_os_socket_addr_family(domain);
    type = newlib_to_os_socket_type(type);
    protocol = newlib_to_os_socket_protocol(protocol);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)socketpair(domain, type, protocol, sv);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SOCKETPAIR Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_LISTEN(struct libc_msg *msg)
{
    // Origin: int listen(int sockfd, int backlog)
    int sockfd = 0;
    int backlog = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_LISTEN Request Failed: <sockfd>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &backlog, sizeof(int)))
    {
        ERR("CMD_SOCK_LISTEN Request Failed: <backlog>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)listen(sockfd, backlog);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_LISTEN Response Failed: <result>\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SHUTDOWN(struct libc_msg *msg)
{
    // Origin: int shutdown(int sockfd, int how)
    int sockfd = 0;
    int how = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_SHUTDOWN Request Failed: <sockfd>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &how, sizeof(int)))
    {
        ERR("CMD_SOCK_SHUTDOWN Request Failed: <how>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    how = newlib_to_os_socket_shutdown_howto(how);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)shutdown(sockfd, how);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SHUTDOWN Response Failed: <result>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SOCKATMARK(struct libc_msg *msg)
{
    // Origin: int sockatmark(int sockfd)
    int sockfd = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKATMARK Request Failed: <sockfd>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)sockatmark(sockfd);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SOCKATMARK Response Failed: <result>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_RECVFROM(struct libc_msg *msg)
{
    // Origin: ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
    bool ret = false;

    int sockfd = 0;
    int flags = 0;
    size_t len = 0;
    struct sockaddr_storage addr, *p_addr = NULL;
    socklen_t addrlen, *p_addrlen = NULL;

    const size_t buf_max_sz = KB(256); // 256 KB limit
    uint8_t buf[buf_max_sz];

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    memset(buf, 0, buf_max_sz);

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: <sockfd>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &flags, sizeof(int)))
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: <flags>\n");
        goto out;
    }

    if (!varbuf_deserialize_size_t(varbuf, &len))
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: <len>\n");
        goto out;
    }

    if (len > buf_max_sz)
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: Invalid <len>\n");
        goto out;
    }

    // Test if <addr> is NULL
    if (varbuf_tryread_nil(varbuf))
    {
        // The serialized <src_addr> must be NULL.
        if (!varbuf_read_nil(varbuf))
            goto out;

        // The serialized <addrlen> must be NULL.
        if (!varbuf_read_nil(varbuf))
            goto out;

        p_addr = NULL;
        p_addrlen = NULL;
    }
    else
    {
        ret = _deserialize_sockaddr(varbuf, &addr);
        if (!ret)
        {
            ERR("CMD_SOCK_RECVFROM Request Failed: <addr>\n");
            goto out;
        }

        if (!_deserialize_socklen_t(varbuf, &addrlen))
        {
            ERR("CMD_SOCK_RECVFROM Request Failed: <addrlen>\n");
            goto out;
        }

        p_addr = &addr;
        p_addrlen = &addrlen;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Translate newlib's socket input constants to OS-specific posix definitions
    flags = newlib_to_os_socket_send_recv_flags(flags);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)recvfrom(sockfd, buf, len, flags, (struct sockaddr *)p_addr, p_addrlen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_RECVFROM Response Failed: <result>\n");

    if (result > 0)
    {
        varbuf_write_data(varbuf, buf, min(len, buf_max_sz));

        if (!_serialize_sockaddr(varbuf, &addr, addrlen))
            ERR("CMD_SOCK_RECVFROM Response Failed: <addr>\n");

        if (!_serialize_socklen_t(varbuf, addrlen))
            ERR("CMD_SOCK_RECVFROM Response Failed: <addrlen>\n");
    }
    else if (result == 0)
    {
        if (!_serialize_sockaddr(varbuf, &addr, addrlen))
            ERR("CMD_SOCK_RECVFROM Response Failed: <addr>\n");

        if (!_serialize_socklen_t(varbuf, addrlen))
            ERR("CMD_SOCK_RECVFROM Response Failed: <addrlen>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#pragma GCC diagnostic pop