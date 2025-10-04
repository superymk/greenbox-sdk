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
#include <string.h>

#include <appddk.h>
#include <app_sp_intfs.h>

#include "config_mgr.h"
#include "sp_control.h"

/*********** Private Functions **********/
static SAP_CONFIG sap_cfg;

static ulong_t flock_fd = 0;

static int _process_instance_lock_grab(void)
{
	char flock_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char* flock_filename = "sl_secapp.lock";

    strncpy(flock_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(flock_filepath, flock_filename);

	return os_file_lock_create(flock_filepath, &flock_fd);
}

static int _process_instance_lock_release(void)
{
	return os_file_lock_release(flock_fd);
}




/*********** Public Functions **********/
/// @brief Callback handler required in SecProcess loader to handle its exceptions
void sp_loader_on_exception(void)
{
	int ret = 0;

	// Terminate the SecProcess
	secapp_exit();
	secapp_unload();

	// Release SecApp loader lock
	ret = _process_instance_lock_release();
	if(ret)
	{
		APP_ERR("Failed to release SecApp loader lock!\n");
	}

    // Finalize log sub-system.
    sp_loader_log_fini();

    exit(0);
}

int main(int argc, char * argv[])
{
	SL_STATUS status = SL_SUCCESS;
	int ret = 0;

	bool secagent_presence = false;
	SP_USTRUCT* sp_params = NULL;

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

	// Grab lock to ensure the singleton instance of the untrusted loader process.
	ret = _process_instance_lock_grab();
	if(ret)
	{
		APP_ERR("Another SecApp loader is running. Exiting!\n");
		ret = 1;
		goto out;
	}

	// Bind to core 2 to prepare SecApp.
	app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_2);

	// Init handler of loader exceptions
	sp_loader_install_exception_handler();
	
	// Read SecApp configurations
	status = loader_params_read(&sap_cfg);
	if (SL_FAIL(status))
    {
        APP_ERR("Read SecApp configuration error! status:%lu\n", status);
        ret = 1;
		goto out;
    }

	// Allocate resource
	ret = secapp_load(&sap_cfg, &sp_params);
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
		ret = 1;
		goto out;
	}

	// Wait to exit
	press_continue("Press to exit SecApp");
	secapp_exit();

	secapp_unload();

	// Release SecApp loader lock
	ret = _process_instance_lock_release();
	if(ret)
	{
		APP_ERR("Failed to release SecApp loader lock in <main>!\n");
	}

    sp_loader_log_fini();
	return 0;


out:
	(void)_process_instance_lock_release();
    sp_loader_log_fini();
	return ret;
}