// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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