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

#define _GNU_SOURCE
#include <app_types.h>
#include <secagent_intfs.h>

#include <stdio.h>
#include <fcntl.h>	// for open
#include <unistd.h> // for close
#include <sys/mman.h> // for mmap
#include <errno.h>	// for errno
#include <stdlib.h> // for malloc, free
#include <string.h> /* memset */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <pthread.h>

#include <execinfo.h>


// #define __USE_GNU
#include <sched.h>
// #undef __USE_GNU

bool os_ioctl(struct tz_cmd_packet* cmd, int* err_no)
{
	/* call kernel module to allocate a physically-contiguous 
	 * non-cacheable physical memory pool */ 
	int fd = 0;
	int ret = 0;
	bool status = false;

    /* Open Device */
    fd = open(TZ_CLIENT_FULL_PATH_DEV_NAME, O_RDWR | O_SYNC);
    if ( fd<0 ) {
        // APP_ERR( "Cannot Open Device: %s\n", TZ_CLIENT_FULL_PATH_DEV_NAME);
		goto out;
    }

	ret = ioctl(fd, 1, cmd);
	if (ret) {
		// APP_ERR( "os_ioctl error with code %d!\n", ret);
		close(fd);
		goto out;
	}
	
	close(fd);
	status = true;

out:
	if(err_no)
		*err_no = ret;

	// APP_ERR("[Superymk] <os_ioctl> Finished cmd:%u\n", cmd->id);
	return status;
}