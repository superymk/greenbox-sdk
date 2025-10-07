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
#include <syslog.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

void  closelog(void)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	
	// Build the request
	req_msg = libc_create_msg(true, CMD_SYSLOG_CLOSE);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, 0, 1000);
	if(!resp_msg)
		goto out;

	// Read result
	varbuf = varbuf_access_start(resp_msg->req_data);
	varbuf_util_read_result(varbuf, NULL, &reent->_errno);
	varbuf_access_finish(varbuf);

out:
	sp_comm_app_free_msg(req_msg, resp_msg);
}

#define OPENLOG_IDENT_MAX_LENGTH	(128)

void  openlog(const char *ident, int logopt, int facilit)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;

	if(!ident)
		return;
	
    // Build the request
	req_msg = libc_create_msg(true, CMD_SYSLOG_OPEN);
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, ident, strnlen(ident, OPENLOG_IDENT_MAX_LENGTH)))
		goto err;
	if(!varbuf_write_data(varbuf, &logopt, sizeof(int)))
		goto err;
	if(!varbuf_write_data(varbuf, &facilit, sizeof(int)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 1000);
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
	return;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	reent->_errno = ENOSYS;
    return;
}

int   setlogmask(int maskpri)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;
	
    // Build the request
	req_msg = libc_create_msg(true, CMD_SYSLOG_SETLOGMASK);
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, &maskpri, sizeof(int)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 1000);
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
    return -1;
}

#define SYSLOG_SYSLOG_MSG_LENGTH_MAX	(1024)
void  syslog(int priority, const char *message, ...)
{
	struct _reent *reent = _REENT;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;
	char* buf = NULL;
	va_list args;

	if(!message)
		return;

	buf = (char*)malloc(SYSLOG_SYSLOG_MSG_LENGTH_MAX);
	if(!buf)
		return;

	// Initialize the va_list variable with the last fixed parameter
    va_start(args, message);

	vsnprintf(buf, SYSLOG_SYSLOG_MSG_LENGTH_MAX, message, args);

	// Clean up the va_list
    va_end(args);
	
    // Build the request
	req_msg = libc_create_msg(true, CMD_SYSLOG_SYSLOG);
	
	// Serialize params
	varbuf = varbuf_access_start(req_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_write_data(varbuf, &priority, sizeof(int)))
		goto err;
	if(!varbuf_write_data(varbuf, buf, strnlen(buf, SYSLOG_SYSLOG_MSG_LENGTH_MAX)))
		goto err;
	msg_data_sz = varbuf_size(varbuf);
	varbuf_access_finish(varbuf);

	free(buf);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 1000);
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
	return;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	if(buf)
		free(buf);
	reent->_errno = ENOSYS;
    return;
}