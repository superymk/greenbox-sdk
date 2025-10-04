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

#include <libc_backend.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#include <ipc/varbuf.h>
#include "_info.h"
#include "inet_ops.h"

size_t do_CMD_INET_ADDR(struct libc_msg *msg)
{
    // Origin: in_addr_t    inet_addr(const char* str)
    char *str = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);

    str = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!str)
    {
        ERR("CMD_INET_ADDR Request Failed: <af>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)inet_addr(str);

out:
    if (str)
        free(str);
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_INET_NTOA(struct libc_msg *msg)
{
    // Origin: char        *inet_ntoa(struct in_addr addr)
    struct in_addr addr = {0};
    char *str = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);

    if (!varbuf_read_fixdata(varbuf, &addr, sizeof(addr)))
    {
        ERR("CMD_FILE_NTOA Request Failed: <addr>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    str = inet_ntoa(addr);
    result = (uintptr_t)str;

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, &result, &newlib_errno);
    if (result)
        varbuf_write_str(varbuf, str, (strlen(str) + 1));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_INET_PTON(struct libc_msg *msg)
{
    // Origin: int    inet_pton(int af, const char *src, void *dst)
    int af = 0;
    char *src = NULL;
    struct in_addr addr;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);

    if (!varbuf_read_fixdata(varbuf, &af, sizeof(int)))
    {
        ERR("CMD_INET_PTON Request Failed: <af>\n");
        goto out;
    }

    src = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!src)
    {
        ERR("CMD_INET_PTON Request Failed: <src>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)inet_pton(af, src, &addr);

out:
    if (src)
        free(src);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_INET_PTON Response Failed: <result>\n");
    if (!varbuf_write_data(varbuf, &addr, sizeof(struct in_addr)))
        ERR("CMD_INET_PTON Response Failed: <addr>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_INET_NTOP(struct libc_msg *msg)
{
    // Origin: const char* inet_ntop(int af, const void *addr, char *buf, socklen_t len);
    int af = 0;
    uint8_t addr[16] = {0}; // <addr> for inet_ntop is 16 bytes maximum.
    char buf[INET6_ADDRSTRLEN + 1] = {0};
    socklen_t len = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Read the request
    varbuf = varbuf_access_start(msg->req_data);

    if (!varbuf_read_fixdata(varbuf, &af, sizeof(int)))
    {
        ERR("CMD_INET_NTOP Request Failed: <af>\n");
        goto out;
    }

    if (af == AF_INET)
    {
        if (!varbuf_read_fixdata(varbuf, &addr, 4))
        {
            ERR("CMD_INET_NTOP Request Failed: <addr>\n");
            goto out;
        }
    }
    else if (af == AF_INET6)
    {
        if (!varbuf_read_fixdata(varbuf, &addr, 16))
        {
            ERR("CMD_INET_NTOP Request Failed: <addr>\n");
            goto out;
        }
    }

    if (!varbuf_read_fixdata(varbuf, &len, sizeof(uint32_t)))
    {
        ERR("CMD_INET_NTOP Request Failed: <len>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)inet_ntop(af, addr, buf, len);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_INET_NTOP Response Failed: <result>\n");
    if (!varbuf_write_str(varbuf, buf, (strnlen(buf, INET6_ADDRSTRLEN) + 1)))
        ERR("CMD_INET_NTOP Response Failed: <buf>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}
