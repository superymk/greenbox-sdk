// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sys/socket.h>
#include <errno.h>
#include <reent.h>
#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

/// @brief Network operations need longer timeout (1 seconds) for slow network.
#define SOCKET_OPS_TIMEOUT_MS (1000 * 1000)

static bool _deserialize_rawdata(varbuf_t *varbuf, void *out_data)
{
    bool status = false;
    uint32_t datasz = 0;
    char *data = NULL;

    // Sanity check
    if (!out_data)
        return false;

    // Get size
    status = varbuf_tryread_datasz(varbuf, &datasz);
    if (!status)
        goto err;

    data = malloc(datasz);
    if (!data)
        goto err;

    status = varbuf_read_fixdata(varbuf, data, datasz);
    if (!status)
        goto err;

    if (out_data)
        memcpy(out_data, data, datasz);

    if (data)
        free(data);

    // On success
    return true;

err:
    if (data)
        free(data);
    return false;
}

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

static bool _serialize_sockaddr(varbuf_t *varbuf, const struct sockaddr_storage *sock, socklen_t addrlen)
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

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_ACCEPT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read responses
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (!_deserialize_sockaddr(varbuf, addr))
        goto err;
    if (!_deserialize_socklen_t(varbuf, addrlen))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_BIND);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!_serialize_sockaddr(varbuf, (struct sockaddr_storage *)addr, addrlen))
        goto err;
    if (!_serialize_socklen_t(varbuf, addrlen))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read responses
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_CONNECT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!_serialize_sockaddr(varbuf, addr, addrlen))
        goto err;
    if (!_serialize_socklen_t(varbuf, addrlen))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read responses
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_GETPEERNAME);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;

    if (!_serialize_socklen_t(varbuf, *addrlen))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read responses
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (!_deserialize_sockaddr(varbuf, addr))
        goto err;
    if (!_deserialize_socklen_t(varbuf, addrlen))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_GETSOCKNAME);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;

    if (!_serialize_socklen_t(varbuf, addrlen))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read responses
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (!_deserialize_sockaddr(varbuf, addr))
        goto err;
    if (!_deserialize_socklen_t(varbuf, addrlen))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int listen(int sockfd, int backlog)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_LISTEN);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &backlog, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_RECV);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_serialize_size_t(varbuf, len))
        goto err;
    if (!varbuf_write_data(varbuf, &flags, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (result != -1)
        varbuf_read_vardata_direct(varbuf, buf, &len);
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (ssize_t)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    void *result_buf = NULL;
    size_t result_buf_size = 0;
    const size_t buf_max_sz = KB(256); // Must be the same between the libc-backend and the newlib extention for SecProcesses.

    // [Limitation] Check: buf must be no bigger than <buf_max_sz> bytes.
    if (len > buf_max_sz)
        goto err;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_RECVFROM);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &flags, sizeof(int)))
        goto err;
    if (!varbuf_serialize_size_t(varbuf, len))
        goto err;

    if(src_addr && addrlen)
    {
        if (!_serialize_sockaddr(varbuf, src_addr, *addrlen))
            goto err;
        if (!_serialize_socklen_t(varbuf, *addrlen))
            goto err;
    }
    else
    {
        // Send <src_addr> as NULL
        if (!varbuf_write_nil(varbuf))
            goto err;

        // Send <addrlen> as NULL
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read responses
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;

    if (result > 0)
    {
        result_buf = varbuf_read_vardata(varbuf, &result_buf_size);
        if (!result_buf)
        {
            goto err;
        }
        memcpy(buf, result_buf, min(result_buf_size, len));
        free(result_buf);

        if (!_deserialize_sockaddr(varbuf, src_addr))
            goto err;
        if (addrlen)
        {
            if (!varbuf_read_fixdata(varbuf, addrlen, sizeof(*addrlen)))
                goto err;
        }
    }
    else if (result == 0)
    {
        if (!_deserialize_sockaddr(varbuf, src_addr))
            goto err;
        if (addrlen)
        {
            if (!varbuf_read_fixdata(varbuf, addrlen, sizeof(*addrlen)))
                goto err;
        }
    }

    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (ssize_t)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return (ssize_t)-1;
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    struct _reent *reent = _REENT;

    LIBC_FATAL_ERR("<recvmsg> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    if (!buf)
        goto err;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_SEND);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_serialize_size_t(varbuf, len))
        goto err;
    if (!varbuf_write_data(varbuf, &flags, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, buf, len))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (ssize_t)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    struct _reent *reent = _REENT;

    LIBC_FATAL_ERR("<sendmsg> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    struct _reent *reent = _REENT;

    LIBC_FATAL_ERR("<sendto> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_GETSOCKOPT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &level, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &optname, sizeof(int)))
        goto err;
    if (!_serialize_socklen_t(varbuf, *optlen))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (!_deserialize_rawdata(varbuf, optval))
        goto err;
    if (!_deserialize_socklen_t(varbuf, optlen))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_SETSOCKOPT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &level, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &optname, sizeof(int)))
        goto err;
    if (!_serialize_socklen_t(varbuf, optlen))
        goto err;
    if (optval)
    {
        if (!varbuf_write_data(varbuf, optval, optlen))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int shutdown(int sockfd, int how)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_SHUTDOWN);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &how, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int socket(int domain, int type, int protocol)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_SOCKET);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &domain, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &type, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &protocol, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int sockatmark(int sockfd)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_SOCKATMARK);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &sockfd, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int socketpair(int domain, int type, int protocol, int sv[2])
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL, *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_SOCK_SOCKETPAIR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &domain, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &type, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &protocol, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &sv[0], sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &sv[1], sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}
