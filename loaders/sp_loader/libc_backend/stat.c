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
#include <sys/stat.h>
#include <syslog.h>

#include <ipc/varbuf.h>
#include "_info.h"
#include "stat_ops.h"

#define FILEPATH_LENGTH_MAX (4096)

size_t do_CMD_STAT_CHMOD(struct libc_msg *msg)
{
    // Origin: int chmod(const char *pathname, mode_t mode);
    uint32_t pathname_len = FILEPATH_LENGTH_MAX;
    mode_t mode;
    char pathname[FILEPATH_LENGTH_MAX] = {0};

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf_read_vardata_direct(varbuf, pathname, (uint32_t *)&pathname_len))
    {
        ERR("CMD_STAT_CHMOD Request Failed: Invalid <pathname>\n");
        goto out;
    }

    if (pathname_len > FILEPATH_LENGTH_MAX)
    {
        ERR("CMD_STAT_CHMOD Request Failed: Invalid <pathname> length\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &mode, sizeof(mode_t)))
    {
        ERR("CMD_STAT_CHMOD Request Failed: Invalid <mode>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = chmod(pathname, mode);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_STAT_CHMOD Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_STAT_FCHMOD(struct libc_msg *msg)
{
    // Origin: int fchmod(int fd, mode_t mode);
    int fd = 0;
    mode_t mode;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf_read_fixdata(varbuf, &fd, sizeof(int)))
    {
        ERR("CMD_STAT_FCHMOD Request Failed: Invalid <mode>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &mode, sizeof(mode_t)))
    {
        ERR("CMD_STAT_FCHMOD Request Failed: Invalid <mode>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);

    // Call the function
    libc_clear_errno();
    result = fchmod(fd, mode);

out:
    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_STAT_FCHMOD Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}