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
#include <sys/ioctl.h>
#include <syslog.h>

#include <ipc/varbuf.h>
#include "_info.h"
#include "stat_ops.h"

size_t do_CMD_IOCTL_ARG1_RET(struct libc_msg *msg)
{
    // Origin: int ioctl_ret_1arg(int fd, int request, void* arg1, size_t arg1_value_size)
    int fd = 0;
    int request = 0;
    void *arg1 = NULL;
    ulong_t arg1_size = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf_read_fixdata(varbuf, &fd, sizeof(int)))
    {
        ERR("CMD_IOCTL_ARG1_RET Request Failed: Invalid <fd>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &request, sizeof(int)))
    {
        ERR("CMD_IOCTL_ARG1_RET Request Failed: Invalid <request>\n");
        goto out;
    }

    if (!varbuf_tryread_nil(varbuf))
    {
        if (!varbuf_read_fixdata(varbuf, &arg1_size, sizeof(ulong_t)))
        {
            ERR("CMD_IOCTL_ARG1_RET Request Failed: Invalid <arg1_size>\n");
            goto out;
        }

        arg1 = malloc(arg1_size);
        if (!arg1)
            goto out;

        if (!varbuf_read_fixdata(varbuf, arg1, arg1_size))
        {
            ERR("CMD_IOCTL_ARG1_RET Request Failed: Invalid <arg1>\n");
            goto out;
        }
    }
    else
    {
        arg1 = NULL;
        arg1_size = 0;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = ioctl(fd, request, arg1);

out:
    if(varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_IOCTL_ARG1_RET Response Failed\n");

    if (arg1)
    {
        if (!varbuf_write_data(varbuf, arg1, arg1_size))
            ERR("CMD_IOCTL_ARG1_RET Response Failed to serialize <arg1> as non-NULL\n");

        free(arg1);
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            ERR("CMD_IOCTL_ARG1_RET Response Failed to serialize <arg1> as NULL\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}