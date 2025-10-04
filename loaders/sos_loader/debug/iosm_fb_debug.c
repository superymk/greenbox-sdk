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

#include <pthread.h>
#include <sl_headers.h>

#include <secsrvs/iosm_fb_loader.h>

static uint32_t secagent_gpu_test_on(void);
static uint32_t secagent_gpu_test_off(void);



/************ Public Functions **********/
void gpu_debug_without_start_secos(void)
{
	secagent_gpu_test_on();

	press_continue("Press to exit");

	secagent_gpu_test_off();
}




/************ Debug Functions **********/
static uint32_t secagent_gpu_test_on(void)
{
    bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_IOSM_FB_TEST_ON;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_INFO("<secagent_gpu_test_on> error with code %d!\n", ret);
		return -1;
	}

	return cmd.arg1;
}

static uint32_t secagent_gpu_test_off(void)
{
    bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_IOSM_FB_TEST_OFF;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_INFO("<secagent_gpu_test_off> error with code %d!\n", ret);
		return -1;
	}

	return cmd.arg1;
}



