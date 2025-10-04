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

/// @brief File operations need longer timeout for slow disks.
#define FILE_OPS_TIMEOUT_MS		(1000 * 60)

#define FILEPATH_LENGTH_MAX (4096)

int chmod(const char *pathname, mode_t mode)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;

	if(!pathname)
		return -1;
	
    // Build the request
	req_msg = libc_create_msg(true, CMD_STAT_CHMOD);
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, pathname, strnlen(pathname, FILEPATH_LENGTH_MAX)))
		goto err;
	if(!varbuf_write_data(varbuf, &mode, sizeof(mode_t)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_util_read_result(varbuf, &result, &reent->_errno))
		goto err;
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);
	return (int)result;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	reent->_errno = ENOSYS;
    return (int)result;
}

int fchmod(int fd, mode_t mode)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;
	
    // Build the request
	req_msg = libc_create_msg(true, CMD_STAT_FCHMOD);
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, &fd, sizeof(int)))
		goto err;
	if(!varbuf_write_data(varbuf, &mode, sizeof(mode_t)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_util_read_result(varbuf, &result, &reent->_errno))
		goto err;
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);
	return (int)result;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	reent->_errno = ENOSYS;
    return (int)result;
}
