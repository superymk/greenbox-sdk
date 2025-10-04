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

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
// #include <sys/time.h>
// #include <sys/times.h>
#include <syslog.h>

#include <ipc/varbuf.h>
#include "_info.h"
#include "syslog_ops.h"

size_t do_CMD_SYSLOG_CLOSE(struct libc_msg *msg)
{
    // Origin: void closelog(void);
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
    closelog();

    // Response
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    varbuf_util_write_result(varbuf, NULL, &newlib_errno);

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#define OPENLOG_IDENT_MAX_LENGTH (128)
size_t do_CMD_SYSLOG_OPEN(struct libc_msg *msg)
{
    // Origin: void openlog(const char *ident, int logopt, int facilit)
    uint32_t ident_len = OPENLOG_IDENT_MAX_LENGTH;
    int logopt = 0;
    int facilit = 0;
    char ident[OPENLOG_IDENT_MAX_LENGTH] = {0};

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf_read_vardata_direct(varbuf, ident, (uint32_t *)&ident_len))
    {
        ERR("CMD_SYSLOG_OPEN Request Failed: Invalid <ident>\n");
        goto out;
    }

    if (ident_len > OPENLOG_IDENT_MAX_LENGTH)
    {
        ERR("CMD_SYSLOG_OPEN Request Failed: Invalid <ident> length\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &logopt, sizeof(int)))
    {
        ERR("CMD_SYSLOG_OPEN Request Failed: Invalid <logopt>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &facilit, sizeof(int)))
    {
        ERR("CMD_SYSLOG_OPEN Request Failed: Invalid <facilit>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    openlog(ident, logopt, facilit);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SYSLOG_OPEN Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_SYSLOG_SETLOGMASK(struct libc_msg *msg)
{
    // Origin: int setlogmask(int maskpri)
    int maskpri = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);

    if (!varbuf_read_fixdata(varbuf, &maskpri, sizeof(int)))
    {
        ERR("CMD_SYSLOG_SETLOGMASK Request Failed: Invalid <maskpri>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = setlogmask(maskpri);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SYSLOG_SETLOGMASK Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

#define SYSLOG_SYSLOG_MSG_LENGTH_MAX (1024)
size_t do_CMD_SYSLOG_SYSLOG(struct libc_msg *msg)
{
    // Origin: void syslog(int priority, const char *message, ...)
    uint32_t buf_len = SYSLOG_SYSLOG_MSG_LENGTH_MAX;
    int priority = 0;
    char buf[SYSLOG_SYSLOG_MSG_LENGTH_MAX] = {0};

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);

    if (!varbuf_read_fixdata(varbuf, &priority, sizeof(int)))
    {
        ERR("CMD_SYSLOG_SYSLOG Request Failed: Invalid <priority>\n");
        goto out;
    }

    if (!varbuf_read_vardata_direct(varbuf, buf, (uint32_t *)&buf_len))
    {
        ERR("CMD_SYSLOG_SYSLOG Request Failed: Invalid <message>\n");
        goto out;
    }

    if (buf_len > SYSLOG_SYSLOG_MSG_LENGTH_MAX)
    {
        ERR("CMD_SYSLOG_SYSLOG Request Failed: Invalid <message> length\n");
        goto out;
    }

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    syslog(priority, "%s", buf);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_SYSLOG_SYSLOG Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}
