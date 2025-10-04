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

#include <app_log.h>
#include <app_stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>

#include <ipc/varbuf.h>
#include "_info.h"
#include "netdb_ops.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"

#define NETDB_MAX_URL_LEN 2083

size_t do_CMD_NETDB_ENDHOSTENT(struct libc_msg *msg)
{
    // Origin: void endhostent(void);

    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
    endhostent();

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, NULL, &newlib_errno))
        ERR("CMD_NETDB_ENDHOSTENT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    return msg_data_sz;
}

size_t do_CMD_NETDB_ENDPROTOENT(struct libc_msg *msg)
{
    // Origin: void endprotoent(void);
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
    endprotoent();

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, NULL, &newlib_errno))
        ERR("CMD_NETDB_ENDPROTOENT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    return msg_data_sz;
}

size_t do_CMD_NETDB_ENDSERVENT(struct libc_msg *msg)
{
    // Origin: void endservent(void);
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
    endservent();

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, NULL, &newlib_errno))
        ERR("CMD_NETDB_ENDSERVENT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    return msg_data_sz;
}

static bool _serialize_hostent(varbuf_t *varbuf, struct hostent *hostent)
{
    uint32_t i = 0;

    // h_name
    if (hostent->h_name)
    {
        if (!varbuf_write_str_smem(varbuf, hostent->h_name, (strlen(hostent->h_name) + 1)))
            goto err;
        else if (!varbuf_write_nil(varbuf))
            goto err;
    }

    // h_aliases[0]
    //// double pointer first
    if (!varbuf_write_data_smem(varbuf, &hostent->h_aliases, sizeof(void *)))
        goto err;

    //// array
    while (hostent->h_aliases[i])
        i++;

    if (!varbuf_write_data_smem(varbuf, &i, sizeof(uint32_t)))
        goto err;
    i = 0;

    while (hostent->h_aliases[i])
    {
        if (!varbuf_write_str_smem(varbuf, hostent->h_aliases[i], (strlen(hostent->h_aliases[i]) + 1)))
            goto err;
        i++;
    }
    i = 0;

    // h_addrtype
    if (!varbuf_write_data_smem(varbuf, &hostent->h_addrtype, sizeof(int)))
        goto err;

    // h_length
    if (!varbuf_write_data_smem(varbuf, &hostent->h_length, sizeof(int)))
        goto err;

    // h_addr_list[0]
    //// double pointer first
    if (!varbuf_write_data_smem(varbuf, &hostent->h_addr_list, sizeof(void *)))
        goto err;

    //// array
    while (hostent->h_addr_list[i])
        i++;

    if (!varbuf_write_data_smem(varbuf, &i, sizeof(uint32_t)))
        goto err;
    i = 0;

    while (hostent->h_addr_list[i])
    {
        if (!varbuf_write_data_smem(varbuf, hostent->h_addr_list[i], sizeof(struct in_addr)))
            goto err;
        i++;
    }
    i = 0;

    // h_name
    if (hostent->h_addr)
    {
        if (!varbuf_write_data_smem(varbuf, hostent->h_addr, sizeof(unsigned int)))
            goto err;
        else if (!varbuf_write_nil(varbuf))
            goto err;
    }

    // On success
    return true;

err:
    return false;
}

size_t do_CMD_NETDB_GETHOSTBYADDR(struct libc_msg *msg)
{
    // Origin: struct hostent  *gethostbyaddr(const void *addr, size_t len, int type)
    char *addr = NULL;
    size_t len = 0;
    int type = 0;
    struct hostent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    addr = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!addr)
    {
        ERR("CMD_NETDB_GETHOSTBYADDR Request Failed: <addr>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &len, sizeof(size_t)))
    {
        ERR("CMD_NETDB_GETHOSTBYADDR Request Failed: <len>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &type, sizeof(int)))
    {
        ERR("CMD_NETDB_GETHOSTBYADDR Request Failed: <type>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = gethostbyaddr(addr, len, type);
    result = (uintptr_t)result_res;

out:
    if (addr)
        free(addr);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETHOSTBYADDR Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_hostent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETHOSTBYADDR Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_GETHOSTBYNAME(struct libc_msg *msg)
{
    // Origin: struct hostent  *gethostbyname(const char *name)
    char *name = NULL;
    struct hostent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    name = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!name)
    {
        ERR("CMD_NETDB_GETHOSTBYNAME Request Failed: <name>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = gethostbyname(name);
    result = (uintptr_t)result_res;

out:
    if (name)
        free(name);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETHOSTBYNAME Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_hostent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETHOSTBYNAME Response Failed: fail to serialize <res>\n");
    }
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

static bool _serialize_protoent(varbuf_t *varbuf, struct protoent *protoent)
{
    uint32_t i = 0;

    // p_name
    if (protoent->p_name)
    {
        if (!varbuf_write_str_smem(varbuf, protoent->p_name, (strlen(protoent->p_name) + 1)))
            goto err;
        else if (!varbuf_write_nil(varbuf))
            goto err;
    }

    // p_aliases[0]
    //// double pointer first
    if (!varbuf_write_data_smem(varbuf, &protoent->p_aliases, sizeof(void *)))
        goto err;

    //// array
    while (protoent->p_aliases[i])
        i++;

    if (!varbuf_write_data_smem(varbuf, &i, sizeof(uint32_t)))
        goto err;
    i = 0;

    while (protoent->p_aliases[i])
    {
        if (!varbuf_write_str_smem(varbuf, protoent->p_aliases[i], (strlen(protoent->p_aliases[i]) + 1)))
            goto err;
        i++;
    }
    i = 0;

    // p_proto
    if (!varbuf_write_data_smem(varbuf, &protoent->p_proto, sizeof(int)))
        goto err;

    // On success
    return true;

err:
    return false;
}

size_t do_CMD_NETDB_GETPROTOBYNAME(struct libc_msg *msg)
{
    // Origin: struct protoent *getprotobyname(const char *name)
    char *name = NULL;
    struct protoent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    name = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!name)
    {
        ERR("CMD_NETDB_GETPROTOBYNAME Request Failed: <name>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = getprotobyname(name);
    result = (uintptr_t)result_res;

out:
    if (name)
        free(name);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETPROTOBYNAME Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_protoent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETPROTOBYNAME Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_GETPROTOBYNUMBER(struct libc_msg *msg)
{
    // Origin: struct protoent *getprotobynumber(int proto)
    int proto = 0;
    struct protoent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &proto, sizeof(int)))
    {
        ERR("CMD_NETDB_GETPROTOBYNUMBER Request Failed: <proto>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = getprotobynumber(proto);
    result = (uintptr_t)result_res;

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETPROTOBYNUMBER Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_protoent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETPROTOBYNUMBER Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_GETPROTOENT(struct libc_msg *msg)
{
    // Origin: struct protoent *getprotoent(void)
    struct protoent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
    result_res = getprotoent();
    result = (uintptr_t)result_res;

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETPROTOENT Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_protoent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETPROTOENT Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

static bool _serialize_servent(varbuf_t *varbuf, struct servent *servent)
{
    uint32_t i = 0;

    // s_name
    if (servent->s_name)
    {
        if (!varbuf_write_str_smem(varbuf, servent->s_name, (strlen(servent->s_name) + 1)))
            goto err;
        else if (!varbuf_write_nil(varbuf))
            goto err;
    }

    // s_aliases[0]
    //// double pointer first
    if (!varbuf_write_data_smem(varbuf, &servent->s_aliases, sizeof(void *)))
        goto err;

    //// array
    while (servent->s_aliases[i])
        i++;

    if (!varbuf_write_data_smem(varbuf, &i, sizeof(uint32_t)))
        goto err;
    i = 0;

    while (servent->s_aliases[i])
    {
        if (!varbuf_write_str_smem(varbuf, servent->s_aliases[i], (strlen(servent->s_aliases[i]) + 1)))
            goto err;
        i++;
    }
    i = 0;

    // s_port
    if (!varbuf_write_data_smem(varbuf, &servent->s_port, sizeof(int)))
        goto err;

    // s_proto
    if (servent->s_proto)
    {
        if (!varbuf_write_str_smem(varbuf, servent->s_proto, (strlen(servent->s_proto) + 1)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    // On success
    return true;

err:
    return false;
}

size_t do_CMD_NETDB_GETSERVBYNAME(struct libc_msg *msg)
{
    // Origin: struct servent  *getservbyname(const char *name, const char *proto)
    char *name = NULL;
    char *proto = NULL;
    struct servent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    name = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!name)
    {
        ERR("CMD_NETDB_GETSERVBYNAME Request Failed: <name>\n");
        goto out;
    }

    proto = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!proto)
    {
        ERR("CMD_NETDB_GETSERVBYNAME Request Failed: <proto>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = getservbyname(name, proto);
    result = (uintptr_t)result_res;

out:
    if (name)
        free(name);
    if (proto)
        free(proto);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETSERVBYNAME Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_servent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETSERVBYNAME Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_GETSERVBYPORT(struct libc_msg *msg)
{
    // Origin: struct servent  *getservbyport(int port, const char *proto)
    int port = 0;
    char *proto = NULL;
    struct servent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &port, sizeof(int)))
    {
        ERR("CMD_NETDB_GETSERVBYPORT Request Failed: <port>\n");
        goto out;
    }

    proto = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!proto)
    {
        ERR("CMD_NETDB_GETSERVBYPORT Request Failed: <proto>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = getservbyport(port, proto);
    result = (uintptr_t)result_res;

out:
    if (proto)
        free(proto);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETSERVBYPORT Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_servent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETSERVBYPORT Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_GETSERVENT(struct libc_msg *msg)
{
    // Origin: struct servent  *getservent(void)
    struct servent *result_res = NULL;
    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
    result_res = getservent();
    result = (uintptr_t)result_res;

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETSERVENT Response Failed\n");
    if (result_res)
    {
        bool status = false;

        status = _serialize_servent(varbuf, result_res);
        if (!status)
            ERR("CMD_NETDB_GETSERVENT Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_SETHOSTENT(struct libc_msg *msg)
{
    // Origin: void sethostent(int stayopen)
    int stayopen = 0;

    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &stayopen, sizeof(int)))
    {
        ERR("CMD_NETDB_SETHOSTENT Request Failed: <stayopen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    sethostent(stayopen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, NULL, &newlib_errno))
        ERR("CMD_NETDB_SETHOSTENT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_SETPROTOENT(struct libc_msg *msg)
{
    // Origin: void setprotoent(int stayopen)
    int stayopen = 0;

    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &stayopen, sizeof(int)))
    {
        ERR("CMD_NETDB_SETPROTOENT Request Failed: <stayopen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    setprotoent(stayopen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, NULL, &newlib_errno))
        ERR("CMD_NETDB_SETPROTOENT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_SETSERVENT(struct libc_msg *msg)
{
    // Origin: void setservent(int stayopen)
    int stayopen = 0;

    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &stayopen, sizeof(int)))
    {
        ERR("CMD_NETDB_SETSERVENT Request Failed: <stayopen>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    setservent(stayopen);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, NULL, &newlib_errno))
        ERR("CMD_NETDB_SETSERVENT Response Failed\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#define GETADDRINFO_RES_MAX_ENTRIES (10)

/// @brief
/// @param varbuf
/// @param addrinfo
/// @return Return true on success. Otherwise false
static bool _serialize_addrinfo(varbuf_t *varbuf, struct addrinfo *addrinfo)
{
    uint32_t i = 0, n = 0;
    struct addrinfo *rp = NULL;

    // Serialize <addrinfo> into array perserving its order

    //// Get elements count
    for (rp = addrinfo; rp != NULL; rp = rp->ai_next)
    {
        n++;
    }

    // We only return the first 20 items to prevent overflow the buffer pointed by <varbuf>.
    n = min(n, GETADDRINFO_RES_MAX_ENTRIES);
    if (!varbuf_write_data_smem(varbuf, &n, sizeof(uint32_t)))
        goto err;
    
    //// Serialize each element
    for (rp = addrinfo; rp != NULL; rp = rp->ai_next, i++)
    {
        // We only return the first 20 items to prevent overflow the buffer pointed by <varbuf>.
        if (i >= GETADDRINFO_RES_MAX_ENTRIES)
            break;

        if (!varbuf_write_data_smem(varbuf, &rp->ai_flags, sizeof(int)))
            goto err;
        if (!varbuf_write_data_smem(varbuf, &rp->ai_family, sizeof(int)))
            goto err;
        if (!varbuf_write_data_smem(varbuf, &rp->ai_socktype, sizeof(int)))
            goto err;
        if (!varbuf_write_data_smem(varbuf, &rp->ai_protocol, sizeof(int)))
            goto err;

        // We use uint32_t instead of socklen_t, see the reason in
        // https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
        if (!varbuf_write_data_smem(varbuf, &rp->ai_addrlen, sizeof(uint32_t)))
            goto err;

        if (rp->ai_addrlen && rp->ai_addr)
        {
            if (!varbuf_write_data_smem(varbuf, rp->ai_addr, rp->ai_addrlen))
                goto err;
        }
        else
        {
            if (!varbuf_write_nil(varbuf))
                goto err;
        }

        if (rp->ai_canonname)
        {
            size_t l_ai_canonname = strnlen(rp->ai_canonname, NETDB_MAX_URL_LEN + 1);

            if (!varbuf_write_str_smem(varbuf, rp->ai_canonname, l_ai_canonname + 1))
                goto err;
        }
        else
        {
            if (!varbuf_write_nil(varbuf))
                goto err;
        }
    }

    // On success
    return true;

err:
    return false;
}

size_t do_CMD_NETDB_GETADDRINFO(struct libc_msg *msg)
{
    // Origin: int getaddrinfo(const char *restrict node,
    //                    const char *restrict service,
    //                    const struct addrinfo *restrict hints,
    //                    struct addrinfo **restrict res);

    char *node = NULL;
    char *service = NULL;
    uint32_t node_len = NETDB_MAX_URL_LEN;
    uint32_t service_len = NETDB_MAX_URL_LEN;
    struct addrinfo hints = {0};
    struct addrinfo *result_res = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_tryread_datasz(varbuf, &node_len))
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: <node_len>\n");
        goto out;
    }

    if (node_len > NETDB_MAX_URL_LEN)
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: Invalid <node_len>\n");
        goto out;
    }

    node = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!node)
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: Invalid <node>\n");
        goto out;
    }

    if (!varbuf_tryread_datasz(varbuf, &service_len))
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: <service_len>\n");
        goto out;
    }

    if (service_len > NETDB_MAX_URL_LEN)
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: Invalid <service_len>\n");
        goto out;
    }

    service = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!service)
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: Invalid <service>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &hints, sizeof(struct addrinfo)))
    {
        ERR("CMD_NETDB_GETADDRINFO Request Failed: Invalid <hints>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)getaddrinfo(node, service, &hints, &result_res);

out:
    if (node)
        free(node);
    if (service)
        free(service);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETADDRINFO Response Failed\n");
    if (result_res)
    {
        // Serialize <result_res>
        bool status = false;
        status = _serialize_addrinfo(varbuf, result_res);

        // Free addrinfo <result_res> after its serialization, so SecProcesses does not need to call their untrusted
        // loaders to free these info in addition to the <getaddrinfo>
        freeaddrinfo(result_res);

        // [Issue 125] De-allocate SecProcess-App shared memory used in previous <varbuf_write_*> on errors of the
        // latest <varbuf_write_*>
        if (!status)
            ERR("CMD_NETDB_GETADDRINFO Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_NETDB_GETNAMEINFO(struct libc_msg *msg)
{
    // Origin: int getnameinfo (const struct sockaddr *__restrict sa, socklen_t addrlen,
    //      char *__restrict host, socklen_t hostlen, char *__restrict serv,
    //      socklen_t servlen, unsigned int flags)
    struct sockaddr sa = {0};
    struct sockaddr *sa_ptr = &sa;
    uint32_t addrlen = 0;
    char host[NI_MAXHOST] = {0};
    char *host_ptr = host;
    uint32_t hostlen = 0;
    char serv[NI_MAXHOST] = {0};
    char *serv_ptr = serv;
    uint32_t servlen = 0;
    unsigned int flags = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);

    //// Get <sa> and <addrlen>
    if (!varbuf_tryread_nil(varbuf))
    {
        if (!varbuf_read_fixdata(varbuf, &addrlen, sizeof(uint32_t)))
        {
            ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <addrlen>\n");
            goto out;
        }

        if (!varbuf_read_fixdata(varbuf, &sa, addrlen))
        {
            ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <sa>\n");
            goto out;
        }

        sa_ptr = &sa;
    }
    else
    {
        sa_ptr = NULL;
        addrlen = 0;
    }

    //// Get <host> and <hostlen>
    if (!varbuf_tryread_nil(varbuf))
    {
        if (!varbuf_read_fixdata(varbuf, &hostlen, sizeof(uint32_t)))
        {
            ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <hostlen>\n");
            goto out;
        }

        if (!varbuf_read_fixdata(varbuf, host, hostlen))
        {
            ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <host>\n");
            goto out;
        }

        host_ptr = host;
    }
    else
    {
        host_ptr = NULL;
        hostlen = 0;
    }

    //// Get <serv> and <servlen>
    if (!varbuf_tryread_nil(varbuf))
    {
        if (!varbuf_read_fixdata(varbuf, &servlen, sizeof(uint32_t)))
        {
            ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <servlen>\n");
            goto out;
        }

        if (!varbuf_read_fixdata(varbuf, serv, servlen))
        {
            ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <serv>\n");
            goto out;
        }

        serv_ptr = serv;
    }
    else
    {
        serv_ptr = NULL;
        servlen = 0;
    }

    /// Get <flags>
    if (!varbuf_read_fixdata(varbuf, &flags, sizeof(unsigned int)))
    {
        ERR("CMD_NETDB_GETNAMEINFO Request Failed: Invalid <flags>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = getnameinfo(sa_ptr, addrlen, host_ptr, hostlen, serv_ptr, servlen, flags);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_NETDB_GETNAMEINFO Response Failed\n");

    if (host_ptr && hostlen)
    {
        if (!varbuf_write_data(varbuf, host, hostlen))
            ERR("CMD_NETDB_GETNAMEINFO Response Failed to serialize <host> as non-NULL\n");
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            ERR("CMD_NETDB_GETNAMEINFO Response Failed to serialize <host> as NULL\n");
    }

    if (serv_ptr && servlen)
    {
        if (!varbuf_write_data(varbuf, serv, servlen))
            ERR("CMD_NETDB_GETNAMEINFO Response Failed to serialize <serv> as non-NULL\n");
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            ERR("CMD_NETDB_GETNAMEINFO Response Failed to serialize <serv> as NULL\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#pragma GCC diagnostic pop