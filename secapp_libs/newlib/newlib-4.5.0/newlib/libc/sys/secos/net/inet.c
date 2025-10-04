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

#include <arpa/inet.h>
#include <errno.h>
#include <reent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

/// @brief Operations need longer timeout (1 seconds) for slow I/O.
#define OPS_TIMEOUT_MS (1000 * 1000)

in_addr_t inet_addr(const char *str)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = 0;
    size_t msg_data_sz = 0;

    if (!str)
        goto err;

    // [TODO] [Ticket 144]
    // Step 1. Check the length of <hostname>
    if (strnlen(str, IP_STRING_LENGTH_MAX + 1) >= IP_STRING_LENGTH_MAX)
        goto err;

    // Build the request
    req_msg = libc_create_msg(true, CMD_INET_ADDR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_str(varbuf, str, strnlen(str, IP_STRING_LENGTH_MAX) + 1))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, OPS_TIMEOUT_MS);
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
    return (in_addr_t)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return (in_addr_t)-1;
}

// [TODO] We need to put it into thread-local storage later
static char _ntoa_buf[18];
char *inet_ntoa(struct in_addr addr)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = 0;
    size_t msg_data_sz = 0;

    // Clear <_ntoa_buf>
    memset(_ntoa_buf, 0, 18);

    // Build the request
    req_msg = libc_create_msg(true, CMD_INET_NTOA);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &addr, sizeof(struct in_addr)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (result)
    {
        size_t len = 18;
        varbuf_read_vardata_direct(varbuf, _ntoa_buf, &len);
    }

    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return _ntoa_buf;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return NULL;
}

int inet_pton(int af, const char *src, void *dst)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = 0;
    size_t msg_data_sz = 0;

    if (!src)
        goto err;

    // [TODO] [Ticket 144]
    // Step 1. Check the length of <hostname>
    if (strnlen(src, IP_STRING_LENGTH_MAX + 1) >= IP_STRING_LENGTH_MAX)
        goto err;

    // Build the request
    req_msg = libc_create_msg(true, CMD_INET_PTON);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_data(varbuf, &af, sizeof(af)))
        goto err;
    if (!varbuf_write_str(varbuf, src, strnlen(src, IP_STRING_LENGTH_MAX) + 1))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (dst)
    {
        if (!varbuf_read_fixdata(varbuf, dst, sizeof(struct in_addr)))
            goto err;
    }
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return -1;
}

const char *inet_ntop(int af, const void *addr, char *buf, socklen_t len)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = 0;
    size_t msg_data_sz = 0;

    void *result_str = NULL;
    size_t result_str_size = 0;

    if (!addr)
        goto err;

    // Build the request
    req_msg = libc_create_msg(true, CMD_INET_NTOP);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_data(varbuf, &af, sizeof(int)))
        goto err;

    if (af == AF_INET)
    {
        if (!varbuf_write_data(varbuf, addr, 4)) // 32 bits for IPv4 numeric addresses
            goto err;
    }
    else if (af == AF_INET6)
    {
        if (!varbuf_write_data(varbuf, addr, 16)) // 128 bits for IPv6 numeric addresses
            goto err;
    }

    if (!varbuf_write_data(varbuf, &len, sizeof(uint32_t))) // Serialize socklen_t as uint32_t
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;

    if (!result)
    {
        // Result is NULL, there must be something wrong.
        goto err;
    }

    result_str = varbuf_read_vardata(varbuf, &result_str_size);
    if (!result_str)
    {
        goto err;
    }

    memcpy(buf, result_str, min(result_str_size, len));
    free(result_str);

    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return buf;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return NULL;
}