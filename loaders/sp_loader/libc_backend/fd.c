// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <unistd.h>
#include <dirent.h>

#include <app_log.h>
#include <ipc/varbuf.h>
#include "_info.h"
#include "fd_ops.h"

#define NEWLIB_FOPEN (-1)               /* from sys/file.h, kernel use only */
#define NEWLIB_FREAD 0x0001             /* read enabled */
#define NEWLIB_FWRITE 0x0002            /* write enabled */
#define NEWLIB_FAPPEND 0x0008           /* append (writes guaranteed at the end) */
#define NEWLIB_FMARK 0x0010             /* internal; mark during gc() */
#define NEWLIB_FDEFER 0x0020            /* internal; defer for next gc pass */
#define NEWLIB_FASYNC 0x0040            /* signal pgrp when data ready */
#define NEWLIB_FSHLOCK 0x0080           /* BSD flock() shared lock present */
#define NEWLIB_FEXLOCK 0x0100           /* BSD flock() exclusive lock present */
#define NEWLIB_FCREAT 0x0200            /* open with file create */
#define NEWLIB_FTRUNC 0x0400            /* open with truncation */
#define NEWLIB_FEXCL 0x0800             /* error on open if file exists */
#define NEWLIB_FNBIO 0x1000             /* non blocking I/O (sys5 style) */
#define NEWLIB_FSYNC 0x2000             /* do all writes synchronously */
#define NEWLIB_FNONBLOCK 0x4000         /* non blocking I/O (POSIX style) */
#define NEWLIB_FNDELAY NEWLIB_FNONBLOCK /* non blocking I/O (4.2 style) */
#define NEWLIB_FNOCTTY 0x8000           /* don't assign a ctty on this open */

#define NEWLIB_O_ACCMODE (NEWLIB_O_RDONLY | NEWLIB_O_WRONLY | NEWLIB_O_RDWR)

/*
 * Flag values for open(2) and fcntl(2)
 * The kernel adds 1 to the open modes to turn it into some
 * combination of FREAD and FWRITE.
 */
#define NEWLIB_O_RDONLY 0 /* +1 == FREAD */
#define NEWLIB_O_WRONLY 1 /* +1 == FWRITE */
#define NEWLIB_O_RDWR 2   /* +1 == FREAD|FWRITE */
#define NEWLIB_O_APPEND NEWLIB_FAPPEND
#define NEWLIB_O_CREAT NEWLIB_FCREAT
#define NEWLIB_O_TRUNC NEWLIB_FTRUNC
#define NEWLIB_O_EXCL NEWLIB_FEXCL
#define NEWLIB_O_SYNC NEWLIB_FSYNC
#define NEWLIB_O_NONBLOCK NEWLIB_FNONBLOCK
#define NEWLIB_O_NOCTTY NEWLIB_FNOCTTY

#define NEWLIB_S_IRWXU 0000700 /* rwx, owner */
#define NEWLIB_S_IRUSR 0000400 /* read permission, owner */
#define NEWLIB_S_IWUSR 0000200 /* write permission, owner */
#define NEWLIB_S_IXUSR 0000100 /* execute/search permission, owner */
#define NEWLIB_S_IRWXG 0000070 /* rwx, group */
#define NEWLIB_S_IRGRP 0000040 /* read permission, group */
#define NEWLIB_S_IWGRP 0000020 /* write permission, grougroup */
#define NEWLIB_S_IXGRP 0000010 /* execute/search permission, group */
#define NEWLIB_S_IRWXO 0000007 /* rwx, other */
#define NEWLIB_S_IROTH 0000004 /* read permission, other */
#define NEWLIB_S_IWOTH 0000002 /* write permission, other */
#define NEWLIB_S_IXOTH 0000001 /* execute/search permission, other */

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

static uint32_t _convert_open_fcntl_flags(uint32_t newlib_flags)
{
    uint32_t result = 0;

    if (newlib_flags & NEWLIB_O_RDONLY)
        result |= O_RDONLY;
    if (newlib_flags & NEWLIB_O_WRONLY)
        result |= O_WRONLY;
    if (newlib_flags & NEWLIB_O_RDWR)
        result |= O_RDWR;
    if (newlib_flags & NEWLIB_O_APPEND)
        result |= O_APPEND;
    if (newlib_flags & NEWLIB_O_CREAT)
        result |= O_CREAT;
    if (newlib_flags & NEWLIB_O_TRUNC)
        result |= O_TRUNC;
    if (newlib_flags & NEWLIB_O_EXCL)
        result |= O_EXCL;
    if (newlib_flags & NEWLIB_O_SYNC)
        result |= O_SYNC;
    if (newlib_flags & NEWLIB_O_NONBLOCK)
        result |= O_NONBLOCK;
    if (newlib_flags & NEWLIB_O_NOCTTY)
        result |= O_NOCTTY;

    return result;
}

static uint32_t _convert_open_mode(uint32_t newlib_flags)
{
    uint32_t result = 0;

    if (newlib_flags & NEWLIB_S_IRWXU)
        result |= S_IRWXU;
    if (newlib_flags & NEWLIB_S_IRUSR)
        result |= S_IRUSR;
    if (newlib_flags & NEWLIB_S_IWUSR)
        result |= S_IWUSR;
    if (newlib_flags & NEWLIB_S_IXUSR)
        result |= S_IXUSR;

    if (newlib_flags & NEWLIB_S_IRWXG)
        result |= S_IRWXG;
    if (newlib_flags & NEWLIB_S_IRGRP)
        result |= S_IRGRP;
    if (newlib_flags & NEWLIB_S_IWGRP)
        result |= S_IWGRP;
    if (newlib_flags & NEWLIB_S_IXGRP)
        result |= S_IXGRP;

    if (newlib_flags & NEWLIB_S_IRWXO)
        result |= S_IRWXO;
    if (newlib_flags & NEWLIB_S_IROTH)
        result |= S_IROTH;
    if (newlib_flags & NEWLIB_S_IWOTH)
        result |= S_IWOTH;
    if (newlib_flags & NEWLIB_S_IXOTH)
        result |= S_IXOTH;

    return result;
}

size_t do_CMD_FILE_OPEN(struct libc_msg *msg)
{
    // Origin: int open(const char *name, int flags, ...)
    char *name = NULL;
    int flags = 0;
    mode_t mode = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    name = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!name)
    {
        ERR("CMD_FILE_OPEN Request Failed: <name>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &flags, sizeof(int)))
    {
        ERR("CMD_FILE_OPEN Request Failed: <flags>\n");
        goto out;
    }
    if (!varbuf_read_fixdata(varbuf, &mode, sizeof(mode_t)))
    {
        ERR("CMD_FILE_OPEN Response Failed: <mode>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)open(name, _convert_open_fcntl_flags(flags),
                            _convert_open_mode(mode));

out:
    if (name)
        free(name);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_OPEN Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_CLOSE(struct libc_msg *msg)
{
    // Origin: int close(int file)
    int file = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &file, sizeof(int)))
    {
        ERR("CMD_FILE_CLOSE Request Failed\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)close(file);
    // APP_ERR("[Superymk] <do_CMD_FILE_CLOSE> file:%d, result:%llu\n", file, result);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_CLOSE Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_READ(struct libc_msg *msg)
{
    // Origin: int read(int file, void *ptr, size_t len)
    int file = 0;
    size_t len = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    char *buf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &file, sizeof(int)))
    {
        ERR("CMD_FILE_READ Request Failed: <file>\n");
        goto out;
    }
    if (!varbuf_read_fixdata(varbuf, &len, sizeof(size_t)))
    {
        ERR("CMD_FILE_READ Request Failed: <len>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    buf = malloc(len);
    memset(buf, 0, len);

    // Call the function
    libc_clear_errno();
    result = (uint64_t)read(file, buf, len);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (varbuf)
    {

        newlib_errno = os_to_newlib_errno(errno);
        if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
            ERR("CMD_FILE_READ Response Failed: <result>\n");

        if (result != -1)
        {
            if (!varbuf_write_data(varbuf, buf, (uint32_t)result))
                ERR("CMD_FILE_READ Response Failed: <buf>\n");
        }
        msg_data_sz = varbuf_size(varbuf);
        varbuf_access_finish(varbuf);
    }
    else
    {
        msg_data_sz = 0;
    }

    if (buf)
        free(buf);

    return msg_data_sz;
}

size_t do_CMD_FILE_WRITE(struct libc_msg *msg)
{
    // Origin: int write(int file, const void *ptr, size_t len)
    int file = 0;
    size_t len = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    char *buf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &file, sizeof(int)))
    {
        ERR("CMD_FILE_WRITE Request Failed\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &len, sizeof(size_t)))
    {
        ERR("CMD_FILE_WRITE Request Failed\n");
        goto out;
    }

    buf = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!buf)
    {
        ERR("CMD_FILE_WRITE Request Failed: <buf>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)write(file, buf, len);

out:
    if (buf)
        free(buf);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_WRITE Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

static void trans_stat_to_nlstat(struct newlib_stat *nlstat, struct stat *st)
{
    nlstat->stat_dev = (uint64_t)st->st_dev;
    nlstat->stat_ino = (uint64_t)st->st_ino;
    nlstat->stat_mode = (uint64_t)st->st_mode;
    nlstat->stat_nlink = (uint64_t)st->st_nlink;
    nlstat->stat_uid = (uint64_t)st->st_uid;
    nlstat->stat_gid = (uint64_t)st->st_gid;
    nlstat->stat_rdev = (uint64_t)st->st_rdev;
    nlstat->stat_size = (uint64_t)st->st_size;
    nlstat->stat_atime = st->st_atime;
    nlstat->stat_mtime = st->st_mtime;
    nlstat->stat_ctime = st->st_ctime;
    nlstat->stat_blksize = (uint64_t)st->st_blksize;
    nlstat->stat_blocks = (uint64_t)st->st_blocks;
}

size_t do_CMD_FILE_FSTAT(struct libc_msg *msg)
{
    // Origin: int fstat(int file, struct stat *st)
    int file = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    struct stat st = {0};
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &file, sizeof(int)))
    {
        ERR("CMD_FILE_FSTAT Request Failed\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)fstat(file, &st);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_FSTAT Response Failed\n");
    if (result != -1)
    {
        struct newlib_stat result_stat = {0};

        trans_stat_to_nlstat(&result_stat, &st);
        varbuf_write_data(varbuf, &result_stat, sizeof(struct newlib_stat));
    }
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_STAT(struct libc_msg *msg)
{
    // Origin: int stat(const char *file, struct stat *st)
    char *file = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    struct stat st = {0};
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    file = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!file)
    {
        ERR("CMD_FILEDIR_STAT Request Failed: <file>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)stat(file, &st);

out:
    if (file)
        free(file);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_CLOSE Response Failed\n");
    if (result != -1)
    {
        struct newlib_stat result_stat = {0};

        trans_stat_to_nlstat(&result_stat, &st);
        varbuf_write_data(varbuf, &result_stat, sizeof(struct newlib_stat));
    }
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_ISATTY(struct libc_msg *msg)
{
    // Origin: int isatty(int file)
    int file = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &file, sizeof(int)))
    {
        ERR("CMD_FILE_ISATTY Request Failed\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)isatty(file);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_ISATTY Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_SELECT(struct libc_msg *msg)
{
    // Origin: int select(int nfds, fd_set *readfds, fd_set *writefds,
    //       fd_set *exceptfds, struct timeval *timeout)
    int nfds = 0;
    fd_set readfds, writefds, exceptfds;
    fd_set *p_readfds, *p_writefds, *p_exceptfds;
    struct timeval timeout;
    struct timeval *p_timeout;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &nfds, sizeof(int)))
    {
        ERR("CMD_FILE_SELECT Request Failed: <nfds>\n");
        goto out;
    }

    if (varbuf_read_fixdata(varbuf, &readfds, sizeof(readfds)))
        p_readfds = &readfds;
    else
        p_readfds = NULL;

    if (varbuf_read_fixdata(varbuf, &writefds, sizeof(writefds)))
        p_writefds = &writefds;
    else
        p_writefds = NULL;

    if (varbuf_read_fixdata(varbuf, &exceptfds, sizeof(exceptfds)))
        p_exceptfds = &exceptfds;
    else
        p_exceptfds = NULL;

    if (varbuf_read_fixdata(varbuf, &timeout, sizeof(timeout)))
        p_timeout = &timeout;
    else
        p_timeout = NULL;

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)select(nfds, p_readfds, p_writefds, p_exceptfds, p_timeout);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_SELECT Response Failed: <result>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_FCNTL(struct libc_msg *msg)
{
    // Origin: int fcntl(int fd, int cmd, int flag)
    int fd = 0;
    int cmd = 0;
    int flag = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &fd, sizeof(int)))
    {
        ERR("CMD_FILE_FCNTL Request Failed: <fd>\n");
        goto out;
    }
    if (!varbuf_read_fixdata(varbuf, &cmd, sizeof(int)))
    {
        ERR("CMD_FILE_FCNTL Request Failed: <cmd>\n");
        goto out;
    }
    if (!varbuf_read_fixdata(varbuf, &flag, sizeof(int)))
    {
        ERR("CMD_FILE_FCNTL Request Failed: <flag>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)fcntl(fd, cmd, _convert_open_fcntl_flags(flag));

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_FCNTL Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_LSEEK(struct libc_msg *msg)
{
    // Origin: _off_t lseek(int file, off_t ptr, int dir)
    int file = 0;
    off_t ptr = 0;
    int dir = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &file, sizeof(int)))
    {
        ERR("CMD_FILE_LSEEK Request Failed: <file>\n");
        goto out;
    }
    if (!varbuf_read_fixdata(varbuf, &ptr, sizeof(ptr)))
    {
        ERR("CMD_FILE_LSEEK Request Failed: <ptr>\n");
        goto out;
    }
    if (!varbuf_read_fixdata(varbuf, &dir, sizeof(int)))
    {
        ERR("CMD_FILE_LSEEK Request Failed: <dir>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)lseek(file, ptr, dir);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_LSEEK Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_POLL(struct libc_msg *msg)
{
    // Origin: int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    unsigned int nfds = 0;
    int timeout = 0;
    struct pollfd *p_fds = NULL;

    const uint32_t nfds_max = 1024; // Must be the same between the libc-backend and the newlib extention for SecProcesses.

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;
    uint32_t i = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &nfds, sizeof(unsigned int)))
    {
        ERR("CMD_FILE_POLL Request Failed: <nfds>\n");
        goto out;
    }

    if (nfds > nfds_max)
    {
        ERR("CMD_FILE_POLL Request Failed: Invalid <nfds>\n");
        goto out;
    }

    p_fds = malloc(nfds * sizeof(struct pollfd));
    if (!p_fds)
    {
        ERR("CMD_FILE_POLL Request Failed: No memory for <p_fds>\n");
        goto out;
    }
    memset(p_fds, 0, sizeof(nfds * sizeof(struct pollfd)));

    // Deserialize <fds>
    for (i = 0; i < nfds; i++)
    {
        if (!varbuf_read_fixdata(varbuf, &p_fds[i], sizeof(struct pollfd)))
        {
            ERR("CMD_FILE_POLL Request Failed: <p_fds>\n");
            goto out;
        }
    }

    if (!varbuf_read_fixdata(varbuf, &timeout, sizeof(int)))
    {
        ERR("CMD_FILE_POLL Request Failed: <nfds>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)poll(p_fds, nfds, timeout);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_POLL Response Failed: <result>\n");

    // Serialize <fds>
    for (i = 0; i < nfds; i++)
    {
        if (!varbuf_write_data(varbuf, &p_fds[i], sizeof(struct pollfd)))
            ERR("CMD_FILE_POLL Response Failed: <p_fds>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    if (p_fds)
        free(p_fds);

    return msg_data_sz;
}

size_t do_CMD_FILE_RENAME(struct libc_msg *msg)
{
    // Origin: int rename(const char *old, const char *new);
    char *old = NULL;
    char *new = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    old = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!old)
    {
        ERR("CMD_FILE_RENAME Request Failed: <old>\n");
        goto out;
    }

    new = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!new)
    {
        ERR("CMD_FILE_RENAME Request Failed: <new>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)rename(old, new);

out:
    if (old)
        free(old);
    if (new)
        free(new);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_RENAME Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILE_ACCESS(struct libc_msg *msg)
{
    // Origin: int access(const char *path, int mode);
    char *path = NULL;
    int mode = 0;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    path = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!path)
    {
        ERR("CMD_FILE_ACCESS Request Failed: <path>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &mode, sizeof(int)))
    {
        ERR("CMD_FILE_ACCESS Request Failed: <mode>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)access(path, mode);

out:
    if (path)
        free(path);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_ACCESS Response Failed\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILEDIR_MKDIR(struct libc_msg *msg)
{
    // Origin: int mkdir(const char *path, mode_t mode);
    char *path = NULL;
    mode_t mode;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    path = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!path)
    {
        ERR("CMD_FILE_MKDIR Request Failed: <path>\n");
        goto out;
    }

    if (!varbuf_read_fixdata(varbuf, &mode, sizeof(unsigned int)))
    {
        ERR("CMD_FILE_MKDIR Request Failed: <mode>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)mkdir(path, mode);

out:
    if (path)
        free(path);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_MKDIR Response Failed: <result>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILEDIR_OPENDIR(struct libc_msg *msg)
{
    // Origin: DIR *opendir(const char *name);
    char *name = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    name = (char *)varbuf_read_vardata(varbuf, NULL);
    if (!name)
    {
        ERR("CMD_FILE_OPENDIR Request Failed: <name>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    result = (uint64_t)opendir(name);

out:
    if (name)
        free(name);

    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_OPENDIR Response Failed: <result>\n");

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

static bool _serialize_dirent(varbuf_t *varbuf, struct dirent *dirent)
{
    const uint32_t readdir_dname_length_max = 256; // Must be the same between the libc-backend and the newlib extention 
                                                // for SecProcesses.

    if (!dirent)
    {
        if (!varbuf_write_nil(varbuf))
            goto err;
    }
    else
    {
        if (!varbuf_write_data(varbuf, &dirent->d_ino, sizeof(uint64_t)))
            goto err;

#ifdef __WINDOWS__
        // Cygwin does not have <d_off> and <d_reclen>. So the function always serializes them as 0.
        {
            uint64_t d_off = 0;
            uint16_t d_reclen = 0;

            if (!varbuf_write_data(varbuf, &d_off, sizeof(uint64_t)))
                goto err;

            if (!varbuf_write_data(varbuf, &d_reclen, sizeof(uint16_t)))
                goto err;
        }
#else
        if (!varbuf_write_data(varbuf, &dirent->d_off, sizeof(uint64_t)))
            goto err;

        if (!varbuf_write_data(varbuf, &dirent->d_reclen, sizeof(uint16_t)))
            goto err;
#endif // __WINDOWS__

        if (!varbuf_write_data(varbuf, &dirent->d_type, sizeof(uint8_t)))
            goto err;

        if (!varbuf_write_str(varbuf, dirent->d_name, (strnlen(dirent->d_name, readdir_dname_length_max) + 1)))
            goto err;
    }

    // On success
    return true;

err:
    return false;
}

size_t do_CMD_FILEDIR_READDIR(struct libc_msg *msg)
{
    // Origin: struct dirent *readdir(DIR *dirp);
    // struct dirent {
    //            ino_t          d_ino;       /* Inode number */
    //            off_t          d_off;       /* Not an offset; see below */
    //            unsigned short d_reclen;    /* Length of this record */
    //            unsigned char  d_type;      /* Type of file; not supported
    //                                           by all filesystem types */
    //            char           d_name[256]; /* Null-terminated filename */
    //        };
    vaddr_t dirp_vaddr = INVALID_VADDR;
    struct dirent *result_res = NULL;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &dirp_vaddr, sizeof(vaddr_t)))
    {
        ERR("CMD_FILE_READDIR Request Failed: <dirp_vaddr>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result_res = readdir((DIR *)dirp_vaddr);
    result = (uintptr_t)result_res;

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILE_READDIR Response Failed: <result>\n");

    // Serialize <result_res>
    {
        bool status = false;

        status = _serialize_dirent(varbuf, result_res);
        if (!status)
            ERR("CMD_FILE_READDIR Response Failed: fail to serialize <res>\n");
    }

    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}

size_t do_CMD_FILEDIR_CLOSEDIR(struct libc_msg *msg)
{
    // Origin: int closedir(DIR *dirp);
    vaddr_t dirp_vaddr = INVALID_VADDR;

    uint64_t result = -1;
    int newlib_errno = 0;
    varbuf_t *varbuf = NULL;
    size_t msg_data_sz = 0;

    varbuf = varbuf_access_start(msg->req_data);
    if (!varbuf)
        goto out;

    if (!varbuf_read_fixdata(varbuf, &dirp_vaddr, sizeof(vaddr_t)))
    {
        ERR("CMD_FILEDIR_CLOSEDIR Request Failed: <dirp_vaddr>\n");
        goto out;
    }

    varbuf_access_finish(varbuf);
    varbuf = NULL;

    // Call the function
    libc_clear_errno();
    result = (uint64_t)closedir((DIR *)dirp_vaddr);

out:
    if (varbuf)
        varbuf_access_finish(varbuf);

    // return the result
    varbuf = varbuf_access_start(msg->resp_data);
    if (!varbuf)
        return 0;

    newlib_errno = os_to_newlib_errno(errno);
    if (!varbuf_util_write_result(varbuf, &result, &newlib_errno))
        ERR("CMD_FILEDIR_CLOSEDIR Response Failed: <result>\n");
    msg_data_sz = varbuf_size(varbuf);
    varbuf_access_finish(varbuf);

    return msg_data_sz;
}