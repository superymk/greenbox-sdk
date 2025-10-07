// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/* note these headers are all provided by newlib - you don't need to provide them */
#include <errno.h>
#include <reent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

/// @brief File operations need longer timeout for slow devices.
#define IOCTL_OPS_TIMEOUT_MS		(1000 * 60)

int ioctl_ret_1arg(int fd, int request, void* arg1, size_t arg1_value_size)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;

	
    // Build the request
	req_msg = libc_create_msg(true, CMD_IOCTL_ARG1_RET);
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, &fd, sizeof(int)))
		goto err;
	if(!varbuf_write_data(varbuf, &request, sizeof(int)))
		goto err;

	if(arg1)
	{
		//// This function serializes <arg1_value_size> as ulong_t (not size_t) for better portability across platforms
		if(!varbuf_write_data(varbuf, &arg1_value_size, sizeof(ulong_t)))
			goto err;

		if(!varbuf_write_data(varbuf, arg1, arg1_value_size))
			goto err;
	}
	else
	{
		if(!varbuf_write_nil(varbuf))
			goto err;
	}
	
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, IOCTL_OPS_TIMEOUT_MS);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_util_read_result(varbuf, &result, &reent->_errno))
		goto err;
	
	//// Get the returned <arg1>
	if(!varbuf_tryread_nil(varbuf))
	{
		if(!varbuf_read_fixdata(varbuf, arg1, arg1_value_size))
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
}