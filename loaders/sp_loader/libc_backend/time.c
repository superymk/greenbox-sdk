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
#include <sys/time.h>
#include <sys/times.h>

#include <ipc/varbuf.h>
#include "_info.h"
#include "time_ops.h"

// [TODO][Issue 102] SecProcesses should not trust time returned by untrusted OS
size_t do_CMD_TIME_GETTIMEOFDAY(struct libc_msg* msg)
{
	// Origin: int gettimeofday(struct timeval *restrict tv, struct timezone *restrict tz);
    bool status = false;
    struct timeval tv;
    struct timezone tz;

	uint64_t result = -1;
    int newlib_errno = 0;
	varbuf_t* varbuf = NULL;
	size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
	result = (uint64_t)gettimeofday(&tv, &tz);

	// Response
	varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
	status = varbuf_util_write_result(varbuf, &result, &newlib_errno);
    if(!status)
    {
        ERR("<do_CMD_TIME_GETTIMEOFDAY> Write <result> error!\n");
    }
    status = varbuf_write_data(varbuf, &tv, sizeof(tv));
    if(!status)
    {
        ERR("<do_CMD_TIME_GETTIMEOFDAY> Write <tv> error!\n");
    }
    status = varbuf_write_data(varbuf, &tz, sizeof(tz));
    if(!status)
    {
        ERR("<do_CMD_TIME_GETTIMEOFDAY> Write <tz> error!\n");
    }
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	return msg_data_sz;
}

// [TODO][Issue 102] SecProcesses should not trust time returned by untrusted OS
size_t do_CMD_TIME_TIMES(struct libc_msg* msg)
{
	// Origin: clock_t times( struct tms* buffer );
    bool status = false;
    struct tms buffer;

	uint64_t result = -1;
    int newlib_errno = 0;
	varbuf_t* varbuf = NULL;
	size_t msg_data_sz = 0;

    // Call the function
    libc_clear_errno();
	result = (uint64_t)times(&buffer);

	// Response
	varbuf = varbuf_access_start(msg->resp_data);
    newlib_errno = os_to_newlib_errno(errno);
	status = varbuf_util_write_result(varbuf, &result, &newlib_errno);
    if(!status)
    {
        ERR("<do_CMD_TIME_TIMES> Write <result> error!\n");
    }
    status = varbuf_write_data(varbuf, &buffer, sizeof(buffer));
    if(!status)
    {
        ERR("<do_CMD_TIME_TIMES> Write <buffer> error!\n");
    }
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	return msg_data_sz;
}