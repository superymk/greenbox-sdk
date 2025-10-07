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

    // sa_family
    status = varbuf_read_fixdata(varbuf, sock, datasz);
    if (!status)
        return false;

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

    // sa_family
    status = varbuf_write_data(varbuf, sock, addrlen);
    if (!status)
        return false;

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
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)accept(sockfd, (struct sockaddr *)&addr, &addrlen);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
    _serialize_sockaddr(varbuf, &addr, addrlen);
    varbuf_write_data(varbuf, &addrlen, sizeof(addrlen));
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
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    _deserialize_sockaddr(varbuf, &addr);
    varbuf_read_fixdata(varbuf, &addrlen, sizeof(addrlen));
    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)bind(sockfd, (struct sockaddr *)&addr, addrlen);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
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
    if(!ret)
    {
        ERR("CMD_SOCK_CONNECT Request Failed: <addr>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &addrlen, sizeof(uint32_t))) // Deserialize <socklen_t> from <uint32_t>
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
    if(varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
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
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)getpeername(sockfd, (struct sockaddr *)&addr, &addrlen);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
    _serialize_sockaddr(varbuf, &addr, addrlen);
    varbuf_write_data(varbuf, &addrlen, sizeof(addrlen));
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

    if (!varbuf_read_fixdata(varbuf, &addrlen, sizeof(uint32_t))) // Deserialize <socklen_t> from <uint32_t>
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
    if(varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);

    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <result>\n");
        
    if (!_serialize_sockaddr(varbuf, &addr, addrlen))
        ERR("CMD_SOCK_GETSOCKNAME Response Failed: <addr>\n");

    if (!varbuf_write_data(varbuf, &addrlen, sizeof(uint32_t)))
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
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    varbuf_read_fixdata(varbuf, &len, sizeof(size_t));
    varbuf_read_fixdata(varbuf, &flags, sizeof(int));
    varbuf_access_finish(varbuf);

    buf = malloc(len);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)recv(sockfd, buf, len, flags);

    // _libc_backend_dbg_print_mem_as_string("CMD_SOCK_RECV", buf, len);
    // APP_ERR("[Superymk] <do_CMD_SOCK_RECV> sockfd:%d, result:0x%llX, os_errno:%d\n", (int)sockfd, (uint64_t)result, (int)errno);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
    varbuf_write_data(varbuf, buf, len);
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    if(buf)
        free(buf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SEND(struct libc_msg *msg)
{
    // Origin: ssize_t send(int sockfd, const void *buf, size_t len, int flags)
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
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    varbuf_read_fixdata(varbuf, &len, sizeof(size_t));
    varbuf_read_fixdata(varbuf, &flags, sizeof(int));

    buf = varbuf_read_vardata(varbuf, NULL);

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)send(sockfd, buf, len, flags);

    // _libc_backend_dbg_print_mem_as_string("CMD_SOCK_SEND", buf, len);
    // APP_ERR("[Superymk] <CMD_SOCK_SEND> sockfd:%d, result:%llu\n", sockfd, result);
    free(buf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
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
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    varbuf_read_fixdata(varbuf, &level, sizeof(int));
    varbuf_read_fixdata(varbuf, &optname, sizeof(int));
    varbuf_read_fixdata(varbuf, &optlen, sizeof(socklen_t));

    // Translate newlib's socket input constants to OS-specific posix definitions
    level = newlib_to_os_socket_option_level(level);
    optname = newlib_to_os_socket_option_flags(optname);

    buf = (char *)malloc((uint32_t)optlen);
    // varbuf_read_vardata_direct(varbuf, buf, (uint32_t*)&optlen);
    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)getsockopt(sockfd, level, optname, buf, &optlen);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
    varbuf_write_data(varbuf, buf, sizeof(optlen));
    varbuf_write_data(varbuf, &optlen, sizeof(optlen));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

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
    char *buf = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);
    varbuf_read_fixdata(varbuf, &sockfd, sizeof(int));
    varbuf_read_fixdata(varbuf, &level, sizeof(int));
    varbuf_read_fixdata(varbuf, &optname, sizeof(int));
    varbuf_read_fixdata(varbuf, &optlen, sizeof(optlen));

    buf = varbuf_read_vardata(varbuf, NULL);

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)setsockopt(sockfd, level, optname, buf, optlen);
    free(buf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
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
    varbuf_read_fixdata(varbuf, &domain, sizeof(int));
    varbuf_read_fixdata(varbuf, &type, sizeof(int));
    varbuf_read_fixdata(varbuf, &protocol, sizeof(int));
    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)socket(domain, type, protocol);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
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
    varbuf_read_fixdata(varbuf, &domain, sizeof(int));
    varbuf_read_fixdata(varbuf, &type, sizeof(int));
    varbuf_read_fixdata(varbuf, &protocol, sizeof(int));
    varbuf_read_fixdata(varbuf, &sv[0], sizeof(int));
    varbuf_read_fixdata(varbuf, &sv[1], sizeof(int));
    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)socketpair(domain, type, protocol, sv);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
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

    // Call the function
    libc_clear_errno();
    result = (uint64_t)listen(sockfd, backlog);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
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

    // Call the function
    libc_clear_errno();
    result = (uint64_t)shutdown(sockfd, how);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_SHUTDOWN Response Failed: <result>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SOCK_SOCKATMARK(struct libc_msg* msg)
{
    // Origin: int sockatmark(int sockfd)
    int sockfd = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);

    if (!varbuf_read_fixdata(varbuf, &sockfd, sizeof(int)))
    {
        ERR("CMD_SOCK_SOCKATMARK Request Failed: <sockfd>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)sockatmark(sockfd);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
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
    struct sockaddr_storage addr;
    socklen_t addrlen;

    const size_t buf_max_sz = 4096;
    uint8_t buf[buf_max_sz];

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    memset(buf, 0, buf_max_sz);

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);

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

    if (!varbuf_read_fixdata(varbuf, &len, sizeof(uint64_t)))
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: <len>\n");
        goto out;
    }

    if (len > buf_max_sz)
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: Invalid <len>\n");
        goto out;
    }

    ret = _deserialize_sockaddr(varbuf, &addr);
    if (!ret)
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: <addr>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &addrlen, sizeof(addrlen)))
    {
        ERR("CMD_SOCK_RECVFROM Request Failed: <addrlen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)recvfrom(sockfd, buf, len, flags, (struct sockaddr *)&addr, &addrlen);

out:
    if(varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SOCK_RECVFROM Response Failed: <result>\n");

    if (result > 0)
    {
        varbuf_write_data(varbuf, buf, min(len, buf_max_sz));

        _serialize_sockaddr(varbuf, &addr, addrlen);
        varbuf_write_data(varbuf, &addrlen, sizeof(addrlen));
    }
    else if (result == 0)
    {
        _serialize_sockaddr(varbuf, &addr, addrlen);
        varbuf_write_data(varbuf, &addrlen, sizeof(addrlen));
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#pragma GCC diagnostic pop