// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <string.h>
#include <errno.h>	// for errno
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include <sl_headers.h>
#include <appddk.h>

#include <secsrv_init_app_intfs.h>

// #include <secsrvs/iosm_pcie_loader.h>

static SP_USTRUCT *init_sp_params = NULL;
static struct sp_resc *init_sp_resc = NULL;

static void _print_welcome_message(void)
{
	SL_INFO("******************************\n");
	SL_INFO("Starting the Init SecProcess\n");
	SL_INFO("******************************\n");
}

static void _secsrv_init_resc(struct sp_resc* resc)
{
    strncpy(resc->filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(resc->filepath, "init.srv");
	resc->dma_size = 0;
	resc->heap_size = KB(256); 
	resc->shared_mem_size = KB(256);
	resc->alloc_sp_app_comm = false;
}

static void _secsrv_prepare_params(SL_CFG* sos_cfg, SP_USTRUCT* params) 
{
	struct init_conf_nonsecure* sp_conf = (struct init_conf_nonsecure*)params->sp_exec_params_nonsecure;

	// Fill the wall clock info
	(void)gettimeofday((struct timeval*)&sp_conf->wall_clock_tv, (struct timezone*)&sp_conf->wall_clock_tz);

	sp_conf->wall_clock_access_tsc = rdtscll();

	params->sp_exec_params_nonsecure_size = sizeof(struct init_conf_nonsecure);
}

/// @brief Allocate resources of the Init SecProcess.
/// Requirement: Must run on a different core from SecOS
/// @param sos_cfg 
/// @param out_params 
/// @param out_resc 
static void init_sp_alloc_rescs(SL_CFG* sos_cfg, SP_USTRUCT** out_params, struct sp_resc** out_resc)
{
	int ret = 0;
	SP_USTRUCT* params = NULL;
	struct sp_resc* resc = NULL;
	SP_SIGNATURE_PARAM sp_sig = {.has_1stparty_signature = true, .sp_signature_relative_filepath = "init.sig"};

	// Check: Parameters must be valid
	if(!sos_cfg || !out_params || !out_resc)
		return;

	_print_welcome_message();

	// Allocate memory
	params = (SP_USTRUCT*)app_mm_alloc(sizeof(SP_USTRUCT));
	if(!params)
	{
		SL_ERR("no memory for <params> of Init SecProcess\n");
		goto err;
	}

	resc = (struct sp_resc*)app_mm_alloc(sizeof(struct sp_resc));
	if(!resc)
	{
		SL_ERR("no memory for <resc> of Init SecProcess\n");
		goto err;
	}

	app_mm_memset(params, 0, sizeof(SP_USTRUCT));
	app_mm_memset(resc, 0, sizeof(struct sp_resc));

	// Init resources
	_secsrv_init_resc(resc);
	_secsrv_prepare_params(sos_cfg, params);
	ret = prepare_secapp(resc, params, &sp_sig);
	if(ret)
		goto err;

	*out_params = params;
	*out_resc = resc;
	return;	
	
err:
	*out_params = NULL;
	*out_resc = NULL;
}

/// @brief Release resources of the Init SecProcess
/// Requirement: Must run on a different core from SecOS
/// @param params 
/// @param resc 
static void init_sp_dealloc_rescs(SP_USTRUCT* params, struct sp_resc* resc)
{
	// Finalize the SecApp
	fini_secapp(resc, params);
}

static int init_sp_register(SP_USTRUCT* params)
{
	int ret = 0;

	ret = apploader_register_secapp(params);
	if(ret)
	{
		SL_ERR("<apploader_register_secapp> error!\n");
		return -1;
	}

	return 0;
}

static int init_sp_unregister(SP_USTRUCT* params)
{
	apploader_unregister_secapp(params);
	return 0;
}

static int init_sp_run(spid_t sp_id)
{
	int ret = 0;
	
	ret = apploader_run_sp(sp_id);
	if(ret)
	{
		SL_ERR("<apploader_run_sp> error!\n");
		return -1;
	}

	return 0;
}

__attribute__((unused)) static int init_sp_terminate(spid_t sp_id)
{
	int ret = 0;
	
	ret = apploader_soft_terminate_sp(sp_id);
	if(ret)
	{
		SL_ERR("<apploader_soft_terminate_sp> error!\n");
		return -1;
	}

	return 0;
}




int _init_sp_load(SL_CFG *sos_cfg)
{
    init_sp_alloc_rescs(sos_cfg, &init_sp_params, &init_sp_resc);
    if (!init_sp_params || !init_sp_resc)
        return 1;

    return 0;
}

int _init_sp_run(void)
{
    int ret = 0;
    bool initialized = false;

    if (!init_sp_params || !init_sp_resc)
        return 1;

    ret = init_sp_register(init_sp_params);
    if (ret)
    {
        SL_ERR("<init_sp_register> error!\n");
        return 1;
    }

    // Update the record of IOSM state in SOS Loader
    sp_states_add(init_sp_params->sp_id, SP_REGISTERED);

    ret = init_sp_run(init_sp_params->sp_id);
    if (ret)
    {
        SL_ERR("<init_sp_run> error!\n");
        return 1;
    }

    // Wait until the IOSM is running
    initialized = sp_block_wait_init(init_sp_params->sp_id, SECSRV_INIT_TIMEOUT_MS);
    if (!initialized)
    {
        SL_ERR("SECSRV-INIT Initialization error or time out!\n");
        return 1;
    }

    // On success
    SL_INFO("SECSRV-INIT is running!\n");
    return 0;
}

int init_sp_unload(void)
{
    int ret = 0;

    if (!init_sp_params || !init_sp_resc)
        return 1;

    // [NOTE] SecOS automatically kills the Init SecProcess when exiting, so this function does not need to request to
    // terminate it again.

    ret = init_sp_unregister(init_sp_params);
    if (ret)
    {
        SL_ERR("<init_sp_unregister> error!\n");
        // [NOTE] We do not return early on error, so the rest of this function can proceed.
    }

    init_sp_dealloc_rescs(init_sp_params, init_sp_resc);

    return 0;
}