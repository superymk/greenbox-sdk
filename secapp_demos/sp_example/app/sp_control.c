// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <string.h>
#include <errno.h>	// for errno
#include <stdlib.h>

#include <appddk.h>
#include <app_sp_intfs.h>
// #include <hid_bridge/hid_ops.h>
// #include <gui_bridge/gui_ops.h>

#include "sp_control.h"

static SP_USTRUCT* sap_params = NULL;
static struct sp_resc* sap_resc = NULL;
static bool sp_exit_unload_called = false;

__attribute__((unused)) static void _print_welcome_message(void)
{
	printf("******************************\n");
	printf("Starting Test SecApp\n");
	printf("******************************\n");
}

__attribute__((unused)) static int _secapp_init_resc(struct sp_resc* resc)
{
    strncpy(resc->filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(resc->filepath, "secapp.sap");
	resc->dma_size = 0;
	resc->heap_size = MB(2);
	resc->shared_mem_size = MB(2);
	
	resc->alloc_sp_app_comm = true;

	// On success
	return 0;
}

__attribute__((unused)) static int _secapp_prepare_params(SP_USTRUCT* params) 
{
	// void* sp_exec_params_secure = params->sp_exec_params_secure;

	return 0;
}

static int _secapp_run(void)
{
	int ret = 0;

	if(!sap_params || !sap_resc)
		return -1;

	// Accept notifications sent from SecAgent
	app_sa_notify_init();

	ret = apploader_register_secapp(sap_params);
	if(ret)
	{
		printf("<apploader_register_secapp> error!\n");
		goto register_sp_err;
	}
	sp_states_add(sap_params->sp_id, SP_REGISTERED);

	ret = apploader_run_sp(sap_params->sp_id);
	if(ret)
	{
		printf("<apploader_run_sp> error!\n");
		return -1;
	}

	// Update the status record of SecProces
    sp_states_set(sap_params->sp_id, SP_RUNNING);

	// On success
	return 0;


register_sp_err:
	// Note: caller will call apploader_unregister_secapp(sap_params);
	return -1;
}

__attribute__((unused)) static int _dtm_get_status(uint64_t* out_dtm_status)
{
	OSSTATUS sa_status;
	
	sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DTM_SECPROCESS_IS_EXIT, out_dtm_status);
	if(SA_FAIL(sa_status))
		return 1;
	else
		return 0;
}

__attribute__((unused)) static void _secapp_terminate_and_unload(void)
{
	int ret = 0;
	SP_STATE state;
    spid_t sp_id;
	uint64_t dtm_status = 0;

	if(!sap_params || !sap_resc)
		return;

	sp_id = sap_params->sp_id;
	ret = apploader_soft_terminate_sp(sp_id);
	if(ret)
	{
		printf("<apploader_soft_terminate_sp> error!\n");
		// [NOTE] We do not return early on error, so the rest of this function can proceed.
	}

	// Wait until the SecProcess exits or the DesktopMgr exits
    while(true)
    {
        msleep(50);

		// If the SecProcess notifies termination, then exit the loop
        state = sp_states_get(sp_id);
		if(state == SP_EXITED)
			break;

		// If the DesktopMgr exits, then exit the loop
		// [NOTE] Needed if the SecProcess depends on DesktopMgr
		ret = _dtm_get_status(&dtm_status);
		if(!ret && (dtm_status == DTM_SECPROCESS_TERMINATED))
			break;
    };

	printf("SecProcess is terminated. Finalizing the SecProcess Loader...\n");
	apploader_unregister_secapp(sap_params);

	// Disable notifications sent from SecAgent
	app_sa_notify_fini();

	printf("SecProcess and its Loader is terminated\n");
}

__attribute__((unused)) static int _secapp_alloc_rescs(SP_USTRUCT** out_params, struct sp_resc** out_resc)
{
	int ret = 0;
	SP_USTRUCT* params = NULL;
	struct sp_resc* resc = NULL;
	SP_SIGNATURE_PARAM sp_sig = {.has_1stparty_signature = false, .sp_signature_relative_filepath = ""};

	_print_welcome_message();

	// Allocate memory
	params = (SP_USTRUCT*)app_mm_alloc(sizeof(SP_USTRUCT));
	if(!params)
	{
		APP_ERR("no memory for <params> of SecApp!\n");
		ret = 1;
		goto err;
	}

	resc = (struct sp_resc*)app_mm_alloc(sizeof(struct sp_resc));
	if(!resc)
	{
		APP_ERR("no memory for <resc> of SecApp!\n");
		ret = 1;
		goto err;
	}

	app_mm_memset(params, 0, sizeof(SP_USTRUCT));
	app_mm_memset(resc, 0, sizeof(struct sp_resc));

	// Init resources
	ret = _secapp_init_resc(resc);
	if(ret)
	{
		APP_ERR("Init SecApp resources error!\n");
		ret = 1;
		goto err;
	}

	ret = _secapp_prepare_params(params);
	if(ret)
	{
		APP_ERR("Prepare parameters error!\n");
		ret = 1;
		goto err;
	}

	ret = prepare_secapp(resc, params, &sp_sig);
	if(ret)
	{
		APP_ERR("Prepare SecApp error!\n");
		ret = 1;
		goto err;
	}

	*out_params = params;
	*out_resc = resc;
	return 0;	
	
err:
	*out_params = NULL;
	*out_resc = NULL;
	return ret;
}

__attribute__((unused)) static void _secapp_dealloc_rescs(SP_USTRUCT* params, struct sp_resc* resc)
{
	// Finalize the SecApp
	fini_secapp(resc, params);
}




/********* Public Functions *********/
int secapp_load(SP_USTRUCT** out_params)
{
	int ret = 0;

	// Init SecProcess loader library.
	ret = sp_loader_init();
    if(ret)
    {
        APP_ERR("<sp_loader_init> error!\n");
		return 1;
    }
	
	// Provision resources of the SecProcess
    ret = _secapp_alloc_rescs(&sap_params, &sap_resc);
	if(ret)
	{
		APP_ERR("<_secapp_alloc_rescs> error!\n");
		return 1;
	}

    *out_params = sap_params;
	return 0;
}

int secapp_run(void)
{
    return _secapp_run();
}

void secapp_exit(void)
{
	if(sp_exit_unload_called)
		return;

	sp_exit_unload_called = true;

	_secapp_terminate_and_unload();
}

void secapp_unload(void)
{
	if(sp_exit_unload_called)
		return;
		
	// Release resource
	_secapp_dealloc_rescs(sap_params, sap_resc);

	// Finalize SecProcess loader library.
	sp_loader_fini();
}


void on_secagent_notify()
{
    OSSTATUS status = OS_SUCCESS;
    struct sa_app_notif_info siginfo;

	// Poll signal's detailed info until there is no more info from SecAgent
    while(status != E_SIG_NO_MORE)
    {
        status = secagent_poll_notification_info(&siginfo);
        if(status != OS_SUCCESS && status != E_SIG_NO_MORE)
        {
            APP_ERR("Failed to poll signal from SecAgent! status:%lu\n", status);
            break;
        }

        // Update SecProcess state according to signal info reported by SecAgent
        if(status == OS_SUCCESS)
        {
            if(siginfo.info1 == SOS_SIGNAL_INFO1_SP_NORMAL_EXIT || 
                siginfo.info1 == SOS_SIGNAL_INFO1_SP_FORCE_EXIT || 
                siginfo.info1 == SOS_SIGNAL_INFO1_SP_EXCEPTION_EXIT)
            {
                // Exit related, so update the corresponding IOSM's state to be exited.
                spid_t sp_id = siginfo.sp_id;
                sp_states_set(sp_id, SP_EXITED);
            }
        }
    }
}