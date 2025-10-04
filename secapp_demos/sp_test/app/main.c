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

#include <stdio.h>
#include <stdlib.h>

#include <appddk.h>
#include <app_sp_intfs.h>

#include "sp_control.h"

/// @brief Callback handler required in SecProcess loader to handle its exceptions
void sp_loader_on_exception(void)
{
	// Terminate the SecProcess
	secapp_exit();
	secapp_unload();

    // Finalize log sub-system.
    sp_loader_log_fini();

    exit(0);
}

int main(int argc, char * argv[])
{
    int ret = 0;
	
	bool secagent_presence = false;
	SP_USTRUCT* sp_params = NULL;

	// parse program input to get trusted path information
	//unsigned int kbd_vid, kbd_pid;

    // Init log subsystem
    ret = sp_loader_log_init(NULL);
    if(ret)
    {
        ret = 1;
        goto out;
    }
    // Loader can use print macros (e.g., APP_INFO, APP_ERR) from now on.

	// Detect the presence of SecAgent
	secagent_presence = secagent_test_presence();
	if(!secagent_presence)
	{
		APP_ERR("SecAgent is not loaded. Exit!\n");
		ret = 1;
        goto out;
	}

	// if (argc!=3) {
	// 	APP_ERR("Please input ./secapp [DEV NUM] [DEV PORT]! argc:%u\n", argc);
	// 	return 0;
	// } 

	// Bind to core 2 to prepare SecApp.
	app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_2);

	// Init handler of loader exceptions
	sp_loader_install_exception_handler();

	// Init SecProcess loader library.
	ret = sp_loader_init();
    if(ret)
    {
        APP_ERR("<sp_loader_init> error!\n");
		ret = 1;
        goto out;
    }

	// Allocate resource
	press_continue("Wait SecOS is running");
	ret = secapp_load(&sp_params);
	if(ret)
	{
		APP_ERR("<secapp_load> error!\n");
		ret = 1;
        goto out;
	}

	// Run the SecApp
	ret = secapp_run();
	if(ret)
	{
		APP_ERR("<secapp_run> error!\n");
	}

	// Wait tests to finish
	// press_continue("Press to exit SecApp");
    APP_INFO("Wait all tests to finish\n");
    while(1)
    {
        if(sp_states_get(sp_params->sp_id) == SP_EXITED)
            break;
    }

	secapp_exit();

	secapp_unload();

    // Finalize SecProcess loader library.
	sp_loader_fini();
	
	// On success
	ret = 0;

out:
    sp_loader_log_fini();
    return ret;
}
