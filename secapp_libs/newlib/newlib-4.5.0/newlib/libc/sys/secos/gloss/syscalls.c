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
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

extern void _do_sp_exit(SP_UTASK *sp_params);

void _exit(int status)
{
	sp_mm_fini();

	pal_mm_destroy_pool();
	libc_fini();
	
    sos_sys_sp_exit((SAP_STATUS)status);
}

char **environ; /* pointer to array of char * strings that define the current environment variables */
int _execve_r(struct _reent * reent, const char *name, char * const argv[], char * const env[])
{
    LIBC_FATAL_ERR("<_execve_r> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

int _fork_r(struct _reent * reent)
{
    LIBC_FATAL_ERR("<_fork_r> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

int _getpid_r(struct _reent * reent)
{
    return (int) sp_get_sapid();
}


int _kill_r(struct _reent * reent, int pid, int sig)
{
    LIBC_FATAL_ERR("<_kill_r> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}

static void* heap = NULL;
static size_t heap_alloc_size = 0;
static size_t heap_max_size = 0;
void* _sbrk_r(struct _reent * reent, ptrdiff_t incr)
{
    void* prevHeap;
    void* nextHeap; 

    if (heap == NULL) 
    {
		// first allocation
		void* heap_base = sp_get_heap_base();
			
		heap = heap_base;
		heap_alloc_size = 0;
		heap_max_size = sp_get_heap_size();
    }

    prevHeap = heap;
            
    nextHeap = (heap + incr);
    heap_alloc_size += incr;

    if(heap_alloc_size <= heap_max_size)
    {
        heap = nextHeap;
        return (void*) prevHeap;    
    }

	reent->_errno = ENOMEM;
	return (void*)(-1);
}

// [TODO][Issue 102] SecProcesses should not trust time returned by untrusted OS
_CLOCK_T_ _times_r(struct _reent * reent, struct tms *buf)
{
    struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;

	// Build the request
	req_msg = libc_create_msg(true, CMD_TIME_TIMES);

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
	if(result != -1)
	{
		if(buf)
			varbuf_read_fixdata(varbuf, buf, sizeof(struct tms));
	}
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);
	return (_CLOCK_T_)result;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	reent->_errno = ENOSYS;
    return -1;
}

int _wait_r(struct _reent * reent, int *status)
{
    LIBC_FATAL_ERR("<_wait_r> Unimplemented\n");
    reent->_errno = ENOSYS;
    return -1;
}



extern VKSTATUS sos_sys_time_init_wall_clock(TIMEVAL* tv, TIMEZONE* tz);
extern VKSTATUS sos_sys_time_get_wall_clock(TIMEVAL* tv, TIMEZONE* tz);


// [TODO][Issue 102] SecProcesses should not trust time returned by untrusted OS
int _gettimeofday_r(struct _reent * reent, struct timeval *p, void* tz)
{
	VKSTATUS status2 = SUCCESS;
	int status = 0;
	TIMEVAL sos_tv;
	TIMEZONE sos_tz;

	// Get the time from SecOS
	status2 = sos_sys_time_get_wall_clock(&sos_tv, &sos_tz);
	if(FAIL(status2))
	{
		reent->_errno = E_SECOS_SYSCALL;
		return -1;
	}

	// Return result
	if(p)
	{
		p->tv_sec = sos_tv.tv_sec;
		p->tv_usec = sos_tv.tv_usec;
	}

	if(tz)
	{
		((struct timezone *)tz)->tz_minuteswest = sos_tz.tz_minuteswest;
		((struct timezone *)tz)->tz_dsttime = sos_tz.tz_dsttime;
	}

	// On success
	return 0;
}






/********* Move to timer.c *********/
extern VKSTATUS sos_sys_time_init_wall_clock(TIMEVAL* tv, TIMEZONE* tz);
extern VKSTATUS sos_sys_time_get_wall_clock(TIMEVAL* tv, TIMEZONE* tz);

int gettimeofday_untrusted(struct timeval *p, void* tz)
{
    struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	varbuf_t* varbuf = NULL;
	uint64_t result = -1;
	size_t msg_data_sz = 0;
	int err = 0;

	// Build the request
	req_msg = libc_create_msg(true, CMD_TIME_GETTIMEOFDAY);

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, msg_data_sz, 1000);
	if(!resp_msg)
		goto err;

	// get result
	varbuf = varbuf_access_start(resp_msg->req_data);
	if(!varbuf)
		goto err;
	if(!varbuf_util_read_result(varbuf, &result, &err))
		goto err;
	if(result != -1)
    {
		if(p)
			varbuf_read_fixdata(varbuf, p, sizeof(struct timeval));
		if(tz)
        	varbuf_read_fixdata(varbuf, tz, sizeof(struct timezone));
    }
	varbuf_access_finish(varbuf);

	sp_comm_app_free_msg(req_msg, resp_msg);
	return (_CLOCK_T_)result;


err:
	sp_comm_app_free_msg(req_msg, resp_msg);
	err = ENOSYS;
    return -1;
}

int secos_init_wall_clock_timer(void)
{
	VKSTATUS status2 = SUCCESS;
	int status = 0;
	struct timeval tv;
	struct timezone tz;
	TIMEVAL sos_tv;
	TIMEZONE sos_tz;

	// Get untrusted OS's current wall clock time
	status = gettimeofday_untrusted(&tv, &tz);
	if(status == -1)
	{
		printf("Adjust SecOS's wall clock timer error: Failed to get untrusted OS's time!\n");
		return -1;
	}

	// Set the wall clock time in SecOS
	sos_tv.tv_sec = tv.tv_sec;
	sos_tv.tv_usec = tv.tv_usec;
	sos_tz.tz_minuteswest = tz.tz_minuteswest;
	sos_tz.tz_dsttime = tz.tz_dsttime;

	status2 = sos_sys_time_init_wall_clock(&sos_tv, &sos_tz);
	if(FAIL(status2))
	{
		printf("Adjust SecOS's wall clock timer error: Failed to set time in SecOS! status:%lu\n", status);
		return -1;
	}

	// On success
	return 0;
}