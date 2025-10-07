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
#include <sys/poll.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

/// @brief File operations need longer timeout (1 seconds) for slow disks.
#define FILE_OPS_TIMEOUT_MS (1000 * 1000)

struct ipc_msg *libc_create_msg(bool req_response, uint32_t cmd)
{
    struct ipc_msg *msg = NULL;

    // Build the request
    msg = sp_comm_app_create_msg(req_response);
    msg->header.cmd_type = LIBC_OPS;
    msg->header.cmd = cmd;

    return msg;
}

int _open_r(struct _reent *reent, const char *name, int flags, int mode)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_OPEN);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_str(varbuf, name, (strlen(name) + 1)))
        goto err;
    if (!varbuf_write_data(varbuf, &flags, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &mode, sizeof(mode_t)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

static int close_file(struct _reent *reent, int file)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_CLOSE);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &file, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

static int read_file(struct _reent *reent, int file, void *ptr, size_t len)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    if (!ptr)
        return -1;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_READ);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &file, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &len, sizeof(size_t)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (result != -1)
        varbuf_read_vardata_direct(varbuf, ptr, &len);
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

static int write_file(struct _reent *reent, int file, const void *ptr, size_t len)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    if (!ptr)
        return -1;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_WRITE);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &file, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &len, sizeof(size_t)))
        goto err;
    if (!varbuf_write_data(varbuf, ptr, len))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

_ssize_t _read_r(struct _reent *reent, int file, void *ptr, size_t len)
{
    int result = -1;

    switch (file)
    {
    case STDIN_FILENO:
    case STDERR_FILENO:
    case STDOUT_FILENO:
        reent->_errno = ENOSYS;
        result = -1;
        break;
    default:
        result = read_file(reent, file, ptr, len);
        break;
    }

    return result;
}

_ssize_t _write_r(struct _reent *reent, int file, const void *ptr, size_t len)
{
    int result = -1;

    switch (file)
    {
    case STDIN_FILENO:
        reent->_errno = ENOSYS;
        result = -1;
        break;
    case STDERR_FILENO:
    case STDOUT_FILENO:
    {
        sos_sys_print_str(ptr, len);
        result = len;
        break;
    }
    default:
        result = write_file(reent, file, ptr, len);
        break;
    }

    return (_ssize_t)result;
}

int _close_r(struct _reent *reent, int file)
{
    int result = -1;

    switch (file)
    {
    case STDIN_FILENO:
    case STDERR_FILENO:
    case STDOUT_FILENO:
        reent->_errno = ENOSYS;
        result = -1;
        break;
    default:
        result = close_file(reent, file);
        break;
    }

    return result;
}

struct newlib_stat
{
    uint64_t stat_dev;
    uint64_t stat_ino;
    uint64_t stat_mode;
    uint64_t stat_nlink;
    uint64_t stat_uid;
    uint64_t stat_gid;
    uint64_t stat_rdev;
    uint64_t stat_size;
    time_t stat_atime;
    uint64_t stat_spare1;
    time_t stat_mtime;
    uint64_t stat_spare2;
    time_t stat_ctime;
    uint64_t stat_spare3;
    uint64_t stat_blksize;
    uint64_t stat_blocks;
    uint64_t stat_spare4[2];
};

static void _time_t_to_timespec(time_t val, struct timespec *out_timespec)
{
    out_timespec->tv_sec = val;
    out_timespec->tv_nsec = 0;
}

static void _deserialize_stat(struct newlib_stat *nlstat, struct stat *out_st)
{
    out_st->st_dev = (dev_t)nlstat->stat_dev;
    out_st->st_ino = (ino_t)nlstat->stat_ino;
    out_st->st_mode = (mode_t)nlstat->stat_mode;
    out_st->st_nlink = (nlink_t)nlstat->stat_nlink;
    out_st->st_uid = (uid_t)nlstat->stat_uid;
    out_st->st_gid = (gid_t)nlstat->stat_gid;
    out_st->st_rdev = (dev_t)nlstat->stat_rdev;
    out_st->st_size = (off_t)nlstat->stat_size;

    _time_t_to_timespec(nlstat->stat_atime, &out_st->st_atim);
    _time_t_to_timespec(nlstat->stat_mtime, &out_st->st_mtim);
    _time_t_to_timespec(nlstat->stat_ctime, &out_st->st_ctim);

    out_st->st_blksize = (blksize_t)nlstat->stat_blksize;
    out_st->st_blocks = (blkcnt_t)nlstat->stat_blocks;
}

int _fstat_r(struct _reent *reent, int file, struct stat *st)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_FSTAT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &file, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (result != -1)
    {
        if (st)
        {
            struct newlib_stat result_stat = {0};

            varbuf_read_fixdata(varbuf, &result_stat, sizeof(struct newlib_stat));

            _deserialize_stat(&result_stat, st);
        }
    }
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_SELECT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &nfds, sizeof(int)))
        goto err;
    if (readfds)
    {
        if (!varbuf_write_data(varbuf, readfds, sizeof(*readfds)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (writefds)
    {
        if (!varbuf_write_data(varbuf, writefds, sizeof(*writefds)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (exceptfds)
    {
        if (!varbuf_write_data(varbuf, exceptfds, sizeof(*exceptfds)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (timeout)
    {
        if (!varbuf_write_data(varbuf, timeout, sizeof(*timeout)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

static int isatty_file(struct _reent *reent, int file)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_ISATTY);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &file, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return 0;
}

int _fcntl_r(struct _reent *reent, int fd, int cmd, int flag)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_FCNTL);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &fd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &cmd, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &flag, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int _isatty_r(struct _reent *reent, int file)
{
    int result = -1;
    switch (file)
    {
    case STDIN_FILENO:
    case STDERR_FILENO:
    case STDOUT_FILENO:
        return 1;
    default:
        result = isatty_file(reent, file);
        break;
    }

    return result;
}

int _stat_r(struct _reent *reent, const char *file, struct stat *st)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_STAT);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (file)
    {
        if (!varbuf_write_str(varbuf, file, (strlen(file) + 1)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    if (result != -1)
    {
        if (st)
        {
            struct newlib_stat result_stat = {0};

            varbuf_read_fixdata(varbuf, &result_stat, sizeof(struct newlib_stat));

            _deserialize_stat(&result_stat, st);
        }
    }
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int _link_r(struct _reent *reent, const char *old, const char *new)
{
    LIBC_FATAL_ERR("<_link_r> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

_off_t _lseek_r(struct _reent *reent, int file, off_t ptr, int dir)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_LSEEK);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &file, sizeof(int)))
        goto err;
    if (!varbuf_write_data(varbuf, &ptr, sizeof(ptr)))
        goto err;
    if (!varbuf_write_data(varbuf, &dir, sizeof(int)))
        goto err;
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (_off_t)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return (_off_t)-1;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;
    uint32_t i = 0;

    const uint32_t nfds_max = 1024; // Must be the same between the libc-backend and the newlib extention for SecProcesses.

    // Check: <nfds> must be not too large.
    if (nfds > nfds_max)
        goto err;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_POLL);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_data(varbuf, &nfds, sizeof(unsigned int)))
        goto err;

    // Serialize <fds>
    FOREACH_S(i, nfds, nfds, 0, 1)
    {
        if (!varbuf_write_data(varbuf, &fds[i], sizeof(struct pollfd)))
            goto err;
    }

    if (!varbuf_write_data(varbuf, &timeout, sizeof(int)))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;

    // Deserialize <fds>
    FOREACH_S(i, nfds, nfds, 0, 1)
    {
        if (!varbuf_read_fixdata(varbuf, &fds[i], sizeof(struct pollfd)))
            goto err;
    }

    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int _unlink_r(struct _reent *reent, const char *name)
{
    LIBC_FATAL_ERR("<_unlink_r> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

int _rename_r(struct _reent *reent, const char *old, const char *new)
{
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_RENAME);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (!varbuf_write_str(varbuf, old, (strlen(old) + 1)))
        goto err;
    if (!varbuf_write_str(varbuf, new, (strlen(new) + 1)))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int access(const char *path, int mode)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILE_ACCESS);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;

    if (path)
    {
        if (!varbuf_write_str(varbuf, path, (strlen(path) + 1)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (!varbuf_write_data(varbuf, &mode, sizeof(int)))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

int mkdir(const char *_path, mode_t __mode)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILEDIR_MKDIR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (_path)
    {
        if (!varbuf_write_str(varbuf, _path, (strlen(_path) + 1)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    if (!varbuf_write_data(varbuf, &__mode, sizeof(unsigned int)))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}

DIR *opendir(const char *dirname)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILEDIR_OPENDIR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (dirname)
    {
        if (!varbuf_write_str(varbuf, dirname, (strlen(dirname) + 1)))
            goto err;
    }
    else
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (DIR *)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return NULL;
}

static void *_deserialize_dirent(varbuf_t *varbuf, struct dirent *out_dirent)
{
    uint64_t d_ino = 0, d_off = 0;
    char *d_name = NULL;

    if (varbuf_tryread_nil(varbuf))
    {
        return NULL;
    }

    if (!varbuf_read_fixdata(varbuf, &d_ino, sizeof(uint64_t)))
        return NULL;
    out_dirent->d_ino = (ino_t)d_ino;

    if (!varbuf_read_fixdata(varbuf, &d_off, sizeof(uint64_t)))
        return NULL;
    out_dirent->d_off = (ino_t)d_off;

    if (!varbuf_read_fixdata(varbuf, &out_dirent->d_reclen, sizeof(uint16_t)))
        return NULL;

    if (!varbuf_read_fixdata(varbuf, &out_dirent->d_type, sizeof(uint8_t)))
        return NULL;

    // d_name
    d_name = varbuf_read_vardata(varbuf, NULL);
    if (!d_name)
        return NULL;

    strncpy(out_dirent->d_name, d_name, sizeof(out_dirent->d_name));
    free(d_name);

    // On success
    return out_dirent;
}

struct dirent *readdir(DIR *dirp)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;
    vaddr_t dirp_vaddr = (vaddr_t)dirp;
    static struct dirent result_res = {0}; // Use static because readdir isn't reentrant.

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILEDIR_READDIR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &dirp_vaddr, sizeof(vaddr_t)))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;

    // De-serialize <result_res>
    {
        void *p;

        p = _deserialize_dirent(varbuf, &result_res);
        if (!p)
            goto err;
    }
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);

    return &result_res;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return NULL;
}

int closedir(DIR *dirp)
{
    struct _reent *reent = _REENT;
    struct ipc_msg *req_msg = NULL;
    struct ipc_msg *resp_msg = NULL;
    varbuf_t *varbuf = NULL;
    uint64_t result = -1;
    size_t msg_data_sz = 0;
    vaddr_t dirp_vaddr = (vaddr_t)dirp;

    // Build the request
    req_msg = libc_create_msg(true, CMD_FILEDIR_CLOSEDIR);

    // Serialize params
    varbuf = varbuf_access_start(req_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_write_data(varbuf, &dirp_vaddr, sizeof(vaddr_t)))
        goto err;

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    // Send the request, and block waiting on the response
    resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, FILE_OPS_TIMEOUT_MS);
    if (!resp_msg)
        goto err;

    // get result
    varbuf = varbuf_access_start(resp_msg->req_data);
    if (!varbuf)
        goto err;
    if (!varbuf_util_read_result(varbuf, &result, &reent->_errno))
        goto err;
    varbuf_access_finish(varbuf);

    sp_comm_app_free_msg(req_msg, resp_msg);
    return (int)result;

err:
    sp_comm_app_free_msg(req_msg, resp_msg);
    reent->_errno = ENOSYS;
    return -1;
}