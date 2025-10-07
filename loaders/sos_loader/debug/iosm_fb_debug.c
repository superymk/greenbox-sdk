// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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



