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

#include <netdb.h>
#include <errno.h>
#include <reent.h>
#include <stddef.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sap_headers_libc.h>
// #include <security/stdlib.h>
#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sys/socket.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

/// @brief Network operations need longer timeout for slow network.
#define SOCKET_OPS_TIMEOUT_MS (1000 * 60)

void endhostent(void)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_ENDHOSTENT);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto out;

    // Read result
    varbuf = varbuf_access_start(resp_msg->req_data);
    varbuf_util_read_result(varbuf, NULL, &reent->_errno);
    varbuf_access_finish(varbuf);

out:
    sp_comm_app_free_msg(req_msg, resp_msg);
}

void endprotoent(void)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_ENDPROTOENT);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto out;

    // Read result
    varbuf = varbuf_access_start(resp_msg->req_data);
    varbuf_util_read_result(varbuf, NULL, &reent->_errno);
    varbuf_access_finish(varbuf);

out:
    sp_comm_app_free_msg(req_msg, resp_msg);
}

void endservent(void)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_ENDSERVENT);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto out;

    // Read result
    varbuf = varbuf_access_start(resp_msg->req_data);
    varbuf_util_read_result(varbuf, NULL, &reent->_errno);
    varbuf_access_finish(varbuf);

out:
    sp_comm_app_free_msg(req_msg, resp_msg);
}

static struct hostent o_hostent = {.h_name = NULL, .h_aliases = NULL, .h_addrtype = 0, .h_length = 0, .h_addr_list = NULL};
static void _clear_hostent(void)
{
    uint32_t i = 0;

    if (o_hostent.h_name)
    {
        free(o_hostent.h_name);
        o_hostent.h_name = NULL;
    }

    if (o_hostent.h_aliases)
    {
        while (o_hostent.h_aliases[i] != NULL)
        {
            free(o_hostent.h_aliases[i]);
            o_hostent.h_aliases[i] = NULL;
            i++;
        }
        i = 0;

        free(o_hostent.h_aliases);
    }

    if (o_hostent.h_addr_list)
    {
        while (o_hostent.h_addr_list[i] != NULL)
        {
            free(o_hostent.h_addr_list[i]);
            o_hostent.h_addr_list[i] = NULL;
            i++;
        }
        i = 0;

        free(o_hostent.h_addr_list);
    }

    o_hostent.h_addrtype = 0;
    o_hostent.h_length = 0;
}

static struct hostent *_deserialize_hostent(uint8_t *resp_data)
{
    uint64_t result = 0;
    varbuf_t *varbuf = NULL;
    struct _reent *reent = _REENT;

    varbuf = varbuf_access_start(resp_data);

    // Read result
    //// return result
    varbuf_util_read_result(varbuf, &result, &reent->_errno);
    if (!result)
        return NULL;

    ////h_name
    {
        o_hostent.h_name = varbuf_read_vardata(varbuf, NULL);
    }

    ////h_aliases[0]
    {
        ////// double pointer first
        varbuf_read_fixdata(varbuf, &result, sizeof(void *));

        if (!result)
            o_hostent.h_aliases = NULL;
        else
        {
            ////// array
            uint32_t i = 0;
            void *src_h_alias_mem;
            uint32_t elems_count = 0;

            varbuf_read_fixdata(varbuf, &elems_count, sizeof(uint32_t));

            src_h_alias_mem = smalloc((elems_count + 1) * sizeof(void *));
            o_hostent.h_aliases = (char **)src_h_alias_mem;
            for (i = 0; i < elems_count; i++)
                o_hostent.h_aliases[i] = varbuf_read_vardata(varbuf, NULL);
            o_hostent.h_aliases[i] = NULL;
        }
    }

    varbuf_read_fixdata(varbuf, &o_hostent.h_addrtype, sizeof(int));
    varbuf_read_fixdata(varbuf, &o_hostent.h_length, sizeof(int));

    ////h_addr_list[0]
    {
        ////// double pointer first
        varbuf_read_fixdata(varbuf, &result, sizeof(void *));

        if (!result)
            o_hostent.h_addr_list = NULL;
        else
        {
            ////// array
            uint32_t i = 0;
            void *src_h_addrlist_mem;
            uint32_t elems_count = 0;

            varbuf_read_fixdata(varbuf, &elems_count, sizeof(uint32_t));

            src_h_addrlist_mem = smalloc((elems_count + 1) * sizeof(void *));
            o_hostent.h_addr_list = (char **)src_h_addrlist_mem;
            for (i = 0; i < elems_count; i++)
                o_hostent.h_addr_list[i] = (char *)varbuf_read_vardata(varbuf, NULL);
            o_hostent.h_addr_list[i] = NULL;
        }
    }

    ////h_addr
    {
        o_hostent.h_addr = varbuf_read_vardata(varbuf, NULL);
    }

    varbuf_access_finish(varbuf);

    return &o_hostent;
}

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    struct hostent *result = NULL;
    size_t msg_data_sz = 0;

    // [TODO] [Ticket 144]
    // Step 1. Check the length of <addr>
    if (len >= (NETDB_MAX_URL_LEN + 1))
        return NULL;

    _clear_hostent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETHOSTBYADDR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_str(varbuf, addr, strnlen(addr, NETDB_MAX_URL_LEN + 1));
    varbuf_write_data(varbuf, &len, sizeof(size_t));
    varbuf_write_data(varbuf, &type, sizeof(int));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct hostent>
    result = _deserialize_hostent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

struct hostent *gethostbyname(const char *name)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    size_t l_hostname = 0;
    varbuf_t *varbuf = NULL;
    struct hostent *result = NULL;
    size_t msg_data_sz = 0;

    // [TODO] [Ticket 144]
    // Step 1. Check the length of <hostname>
    l_hostname = strnlen(name, NI_MAXHOST + 1);
    if (l_hostname >= (NI_MAXHOST + 1))
        return NULL;

    _clear_hostent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETHOSTBYNAME);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_str(varbuf, name, l_hostname + 1);
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct hostent>
    result = _deserialize_hostent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

static struct protoent o_protoent = {.p_name = NULL, .p_aliases = NULL, .p_proto = 0};
static void _clear_protoent(void)
{
    uint32_t i = 0;

    if (o_protoent.p_name)
    {
        free(o_protoent.p_name);
        o_protoent.p_name = NULL;
    }

    if (o_protoent.p_aliases)
    {
        while (o_protoent.p_aliases[i] != NULL)
        {
            free(o_protoent.p_aliases[i]);
            o_protoent.p_aliases[i] = NULL;
            i++;
        }
        i = 0;

        free(o_protoent.p_aliases);
    }

    o_protoent.p_proto = 0;
}

static struct protoent *_deserialize_protoent(uint8_t *resp_data)
{
    uint64_t result = 0;
    varbuf_t *varbuf = NULL;
    struct _reent *reent = _REENT;

    varbuf = varbuf_access_start(resp_data);

    // Read result
    //// return result
    varbuf_util_read_result(varbuf, &result, &reent->_errno);
    if (!result)
        return NULL;

    ////p_name
    {
        o_protoent.p_name = varbuf_read_vardata(varbuf, NULL);
    }

    ////p_aliases[0]
    {
        ////// double pointer first
        varbuf_read_fixdata(varbuf, &result, sizeof(void *));

        if (!result)
            o_protoent.p_aliases = NULL;
        else
        {
            ////// array
            uint32_t i = 0;
            void *src_p_alias_mem;
            uint32_t elems_count = 0;

            varbuf_read_fixdata(varbuf, &elems_count, sizeof(uint32_t));

            src_p_alias_mem = smalloc((elems_count + 1) * sizeof(void *));
            o_protoent.p_aliases = (char **)src_p_alias_mem;
            for (i = 0; i < elems_count; i++)
                o_protoent.p_aliases[i] = varbuf_read_vardata(varbuf, NULL);
            o_protoent.p_aliases[i] = NULL;
        }
    }

    varbuf_read_fixdata(varbuf, &o_protoent.p_proto, sizeof(int));

    varbuf_access_finish(varbuf);

    return &o_protoent;
}

struct protoent *getprotobyname(const char *name)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    size_t l_name = 0;
    varbuf_t *varbuf = NULL;
    struct protoent *result = NULL;
    size_t msg_data_sz = 0;

    // Step 1. Check the length of <name>
    l_name = strnlen(name, NI_MAXHOST + 1);
    if (l_name >= (NI_MAXHOST + 1))
        return NULL;

    _clear_protoent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETPROTOBYNAME);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_str(varbuf, name, l_name + 1);
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct netent>
    result = _deserialize_protoent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

struct protoent *getprotobynumber(int proto)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    struct protoent *result = NULL;
    size_t msg_data_sz = 0;

    _clear_protoent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETPROTOBYNUMBER);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_data(varbuf, &proto, sizeof(int));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct netent>
    result = _deserialize_protoent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

struct protoent *getprotoent(void)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    struct protoent *result = NULL;

    _clear_protoent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETPROTOENT);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct netent>
    result = _deserialize_protoent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

static struct servent o_servent = {.s_name = NULL, .s_aliases = NULL, .s_port = 0, .s_proto = NULL};
static void _clear_servent(void)
{
    uint32_t i = 0;

    if (o_servent.s_name)
    {
        free(o_servent.s_name);
        o_servent.s_name = NULL;
    }

    if (o_servent.s_aliases)
    {
        while (o_servent.s_aliases[i] != NULL)
        {
            free(o_servent.s_aliases[i]);
            o_servent.s_aliases[i] = NULL;
            i++;
        }
        i = 0;

        free(o_servent.s_aliases);
    }

    if (o_servent.s_proto)
    {
        free(o_servent.s_proto);
        o_servent.s_proto = NULL;
    }

    o_servent.s_port = 0;
}

static struct servent *_deserialize_servent(uint8_t *resp_data)
{
    uint64_t result = 0;
    varbuf_t *varbuf = NULL;
    struct _reent *reent = _REENT;

    varbuf = varbuf_access_start(resp_data);

    // Read result
    //// return result
    varbuf_util_read_result(varbuf, &result, &reent->_errno);
    if (!result)
        return NULL;

    ////s_name
    {
        o_servent.s_name = varbuf_read_vardata(varbuf, NULL);
    }

    ////s_aliases[0]
    {
        ////// double pointer first
        varbuf_read_fixdata(varbuf, &result, sizeof(void *));

        if (!result)
            o_servent.s_aliases = NULL;
        else
        {
            ////// array
            uint32_t i = 0;
            void *src_s_alias_mem;
            uint32_t elems_count = 0;

            varbuf_read_fixdata(varbuf, &elems_count, sizeof(uint32_t));

            src_s_alias_mem = smalloc((elems_count + 1) * sizeof(void *));
            o_servent.s_aliases = (char **)src_s_alias_mem;
            for (i = 0; i < elems_count; i++)
                o_servent.s_aliases[i] = varbuf_read_vardata(varbuf, NULL);
            o_servent.s_aliases[i] = NULL;
        }
    }

    varbuf_read_fixdata(varbuf, &o_servent.s_port, sizeof(int));

    ////s_proto
    {
        o_servent.s_proto = varbuf_read_vardata(varbuf, NULL);
    }

    varbuf_access_finish(varbuf);

    return &o_servent;
}

struct servent *getservbyname(const char *name, const char *proto)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    size_t l_name = 0;
    size_t l_proto = 0;
    varbuf_t *varbuf = NULL;
    struct servent *result = NULL;
    size_t msg_data_sz = 0;

    // Step 1. Check the length of <name> and <proto>
    l_name = strnlen(name, NI_MAXHOST + 1);
    if (l_name >= (NI_MAXHOST + 1))
        return NULL;

    l_proto = strnlen(proto, NI_MAXSERV + 1);
    if (l_proto >= (NI_MAXSERV + 1))
        return NULL;

    _clear_servent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETSERVBYNAME);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_str(varbuf, name, l_name + 1);
    varbuf_write_str(varbuf, proto, l_proto + 1);
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct servent>
    result = _deserialize_servent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

struct servent *getservbyport(int port, const char *proto)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    size_t l_proto = 0;
    varbuf_t *varbuf = NULL;
    struct servent *result = NULL;
    size_t msg_data_sz = 0;

    // Step 1. Check the length of <proto>
    l_proto = strnlen(proto, NI_MAXSERV + 1);
    if (l_proto >= (NI_MAXSERV + 1))
        return NULL;

    _clear_servent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETSERVBYPORT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_data(varbuf, &port, sizeof(int));
    varbuf_write_str(varbuf, proto, l_proto + 1);
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct servent>
    result = _deserialize_servent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

struct servent *getservent(void)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    struct servent *result = NULL;

    _clear_servent();

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETSERVENT);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response to build <struct servent>
    result = _deserialize_servent(resp_msg->req_data);
    sp_comm_app_free_msg(req_msg, resp_msg);

    return result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    return NULL;
}

void sethostent(int stayopen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_SETHOSTENT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_data(varbuf, &stayopen, sizeof(int));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto out;

    // Read result
    varbuf = varbuf_access_start(resp_msg->req_data);
    varbuf_util_read_result(varbuf, NULL, &reent->_errno);
    varbuf_access_finish(varbuf);

out:
    sp_comm_app_free_msg(req_msg, resp_msg);
}

void setprotoent(int stayopen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_SETPROTOENT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_data(varbuf, &stayopen, sizeof(int));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto out;

    // Read result
    varbuf = varbuf_access_start(resp_msg->req_data);
    varbuf_util_read_result(varbuf, NULL, &reent->_errno);
    varbuf_access_finish(varbuf);

out:
    sp_comm_app_free_msg(req_msg, resp_msg);
}

void setservent(int stayopen)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_SETSERVENT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    varbuf_write_data(varbuf, &stayopen, sizeof(int));
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto out;

    // Read result
    varbuf = varbuf_access_start(resp_msg->req_data);
    varbuf_util_read_result(varbuf, NULL, &reent->_errno);
    varbuf_access_finish(varbuf);

out:
    sp_comm_app_free_msg(req_msg, resp_msg);
}

#define GETADDRINFO_RES_MAX_ENTRIES (10)
static bool _deserialize_addrinfo(uint8_t *resp_data, int *out_result, struct addrinfo **out_addrinfo)
{
    uint64_t result = 0;
    varbuf_t *varbuf = NULL;
    struct _reent *reent = _REENT;
    uint32_t elems_count = 0;
    uint32_t i = 0;
    struct addrinfo *last_addrinfo = NULL;
    struct addrinfo *first_addrinfo = NULL;

    varbuf = varbuf_access_start(resp_data);

    // Read result
    //// Return result if it is non-zero (i.e., an error happens).
    varbuf_util_read_result(varbuf, &result, &reent->_errno);
    if (result)
    {
        *out_result = result;
        return true;
    }

    //// De-serialize result <res> array
    if (!varbuf_read_fixdata(varbuf, &elems_count, sizeof(uint32_t)))
        goto err;

    if (elems_count > GETADDRINFO_RES_MAX_ENTRIES)
        goto err;

    FOREACH_S(i, elems_count, GETADDRINFO_RES_MAX_ENTRIES, 0, 1)
    {
        struct addrinfo *addrinfo = NULL;

        addrinfo = (struct addrinfo *)malloc(sizeof(struct addrinfo));
        if (!addrinfo)
            goto err;

        if (!varbuf_read_fixdata(varbuf, &addrinfo->ai_flags, sizeof(int)))
            goto err;
        if (!varbuf_read_fixdata(varbuf, &addrinfo->ai_family, sizeof(int)))
            goto err;
        if (!varbuf_read_fixdata(varbuf, &addrinfo->ai_socktype, sizeof(int)))
            goto err;
        if (!varbuf_read_fixdata(varbuf, &addrinfo->ai_protocol, sizeof(int)))
            goto err;

        //// libc_backend serialize <ai_addrlen> as uint32_t (not socklen_t) for better portability across platforms
        if (!varbuf_read_fixdata(varbuf, &addrinfo->ai_addrlen, sizeof(uint32_t)))
            goto err;

        if (!varbuf_tryread_nil(varbuf))
        {
            void *ai_addr = NULL;
            uint32_t ai_addrlen = addrinfo->ai_addrlen;

            ai_addr = malloc(ai_addrlen);
            if (!ai_addr)
                goto err;

            if (!varbuf_read_fixdata(varbuf, ai_addr, ai_addrlen))
                goto err;

            addrinfo->ai_addr = ai_addr;
        }
        else
        {
            addrinfo->ai_addr = NULL;
            if (!varbuf_read_nil(varbuf))
                goto err;
        }

        if (!varbuf_tryread_nil(varbuf))
        {
            char *ai_canonname = NULL;
            uint32_t canonname_len = 0;

            if (!varbuf_tryread_datasz(varbuf, &canonname_len))
                goto err;
            if (canonname_len > NETDB_MAX_URL_LEN)
                goto err;

            ai_canonname = (char *)malloc(canonname_len);
            if (!ai_canonname)
                goto err;

            if (!varbuf_read_fixdata(varbuf, ai_canonname, canonname_len))
                goto err;

            addrinfo->ai_canonname = ai_canonname;
        }
        else
        {
            addrinfo->ai_canonname = NULL;
            if (!varbuf_read_nil(varbuf))
                goto err;
        }

        // Set <first_addrinfo> to be the first <addrinfo>
        if (!first_addrinfo)
            first_addrinfo = addrinfo;

        // Link <addrinfo> with <last_addrinfo> thru the <ai_next> field
        if (last_addrinfo)
        {
            last_addrinfo->ai_next = addrinfo;
        }
        last_addrinfo = addrinfo;
    }

    // Set the last addrinfo's <ai_next> to be NULL;
    last_addrinfo->ai_next = NULL;

    varbuf_access_finish(varbuf);

    // On success
    *out_result = result;
    *out_addrinfo = first_addrinfo;
    return true;

err:
    varbuf_access_finish(varbuf);
    *out_result = NULL;
    *out_addrinfo = NULL;
    return false;
}

int getaddrinfo(const char *restrict node,
                const char *restrict service,
                const struct addrinfo *restrict hints,
                struct addrinfo **restrict res)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;

    size_t l_node = 0;
    size_t l_service = 0;

    varbuf_t *varbuf = NULL;
    int result = -1;
    struct addrinfo *result_addrinfo = NULL;
    size_t msg_data_sz = 0;
    bool ret = false;

    // Check the length of <node>
    l_node = strnlen(node, NETDB_MAX_URL_LEN);
    if (l_node > NETDB_MAX_URL_LEN)
        return -1;

    // Check the length of <service>
    l_service = strnlen(service, NETDB_MAX_URL_LEN);
    if (l_service > NETDB_MAX_URL_LEN)
        return -1;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETADDRINFO);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_str(varbuf, node, l_node + 1))
        goto err;
    if (!varbuf_write_str(varbuf, service, l_service + 1))
        goto err;
    if (!varbuf_write_data(varbuf, hints, sizeof(struct addrinfo)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, SOCKET_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // Read response
    ret = _deserialize_addrinfo(resp_msg->req_data, &result, &result_addrinfo);
    if (!ret)
    {
        result = -1;
        goto err;
    }

    // On success
    if (res)
        *res = result_addrinfo;

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return (int)result;
}

void freeaddrinfo(struct addrinfo *res)
{
    struct addrinfo *rp = res;

    if (!res)
        return;

    do
    {
        struct addrinfo *ai_next = rp->ai_next;

        if (rp->ai_addr)
        {
            free(rp->ai_addr);
            rp->ai_addr = NULL;
        }

        if (rp->ai_canonname)
        {
            free(rp->ai_canonname);
            rp->ai_canonname = NULL;
        }

        free(rp);

        rp = ai_next;
    } while (rp != NULL);
}

const char *gai_strerror(int errcode)
{
    switch (errcode)
    {
    case EAI_BADFLAGS:
        return "flags parameter had an invalid value";

    case EAI_NONAME:
        return "name does not resolve";

    case EAI_AGAIN:
        return "name could not be resolved at this time";

    case EAI_FAIL:
        return "non-recoverable failure in name resolution";

    case EAI_NODATA:
        return "No address associated with NAME.";

    case EAI_FAMILY:
        return "address family not recognized";

    case EAI_SOCKTYPE:
        return "intended socket type was not recognized";

    case EAI_SERVICE:
        return "service not recognized for socket type";

    case EAI_ADDRFAMILY:
        return "address family for NAME not supported";

    case EAI_MEMORY:
        return "memory allocation failure";

    case EAI_SYSTEM:
        return "system error returned in errno";

    case EAI_OVERFLOW:
        return "argument buffer overflow";

#ifdef __USE_GNU
    case EAI_INPROGRESS:
        return "Processing request in progress.";

    case EAI_CANCELED:
        return "Request canceled. ";

    case EAI_NOTCANCELED:
        return "Request not canceled.";

    case EAI_ALLDONE:
        return "All requests done.";

    case EAI_INTR:
        return "Interrupted by a signal.";

    case EAI_IDN_ENCODE:
        return "IDN encoding failed.";
#endif

    default:
        return "unrecognized error code";
    }
}

int getnameinfo(const struct sockaddr *__restrict sa, socklen_t addrlen,
                char *__restrict host, socklen_t hostlen, char *__restrict serv,
                socklen_t servlen, unsigned int flags)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_NETDB_GETNAMEINFO);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (sa)
    {
        //// This function serializes <addrlen> as uint32_t (not socklen_t) for better portability across platforms
        if (!varbuf_write_data(varbuf, &addrlen, sizeof(uint32_t)))
            goto err;

        if (!varbuf_write_data(varbuf, sa, addrlen))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (host)
    {
        //// This function serializes <hostlen> as uint32_t (not socklen_t) for better portability across platforms
        if (!varbuf_write_data(varbuf, &hostlen, sizeof(uint32_t)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (serv)
    {
        //// This function serializes <servlen> as uint32_t (not socklen_t) for better portability across platforms
        if (!varbuf_write_data(varbuf, &servlen, sizeof(uint32_t)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (!varbuf_write_data(varbuf, &flags, sizeof(unsigned int)))
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

    //// Get the returned <host>
    if (!varbuf_tryread_nil(varbuf))
    {
        if (!varbuf_read_fixdata(varbuf, host, hostlen))
        {
            result = -1;
            goto err;
        }
    }

    //// Get the returned <serv>
    if (!varbuf_tryread_nil(varbuf))
    {
        if (!varbuf_read_fixdata(varbuf, serv, servlen))
        {
            result = -1;
            goto err;
        }
    }

    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return (int)result;
};