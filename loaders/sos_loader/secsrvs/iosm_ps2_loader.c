// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <string.h>
#include <errno.h>	// for errno
#include <pthread.h>
#include <stdlib.h>
#include <sl_headers.h>

#include <secsrvs/iosm_ps2_loader.h>

static uint32_t secagent_ps2_drain_buffers(void)
{
	OSSTATUS sa_status = OS_SUCCESS;
	bool status = false;
	struct tz_cmd_packet cmd;
	int ret = 0;
	
	cmd.id = CMD_IOSM_PS2_DRAIN_BUFFERS;
	status = os_ioctl(&cmd, &ret);
	if(!status)
	{
		SL_INFO("<secagent_ps2_drain_buffers> error with code %d!\n", ret);
		return 1;
	}

	sa_status = cmd.status;
	if(SA_FAIL(sa_status))
	{
		SL_INFO("<secagent_ps2_drain_buffers> SecAgent error: %d!\n", sa_status);
		return 1;
	}

	return 0;
}

static uint32_t secagent_resume_ps2_devs(void)
{
	uint32_t status = 0, status2 = 0; 
	uint32_t intr_irq1 = 0, intr_irq12 = 0;
	uint32_t i = 0;
	uint32_t num_cpus = app_cpus_get_procs_num();

	status2 = secagent_x86_os_irq_get_intr_vector(1, &intr_irq1);
	if(status2)
	{
		SL_ERR("Get the interrupt vector of IRQ1 error! status:%lu\n", status2);
		status = 1;
	}
	status2 = secagent_x86_os_irq_get_intr_vector(12, &intr_irq12);
	if(status2)
	{
		SL_ERR("Get the interrupt vector of IRQ12 error! status:%lu\n", status2);
		status = 1;
	}

	for(i = 0; i < num_cpus; i++)
	{
		app_set_cpu_affinity(i);
		(void)secagent_os_trigger_intr(intr_irq1);
		(void)secagent_os_trigger_intr(intr_irq12);
	}

	return status;
}

static void _print_welcome_message(void)
{
	SL_INFO("******************************\n");
	SL_INFO("Starting the IOSM-PS2 Service\n");
	SL_INFO("******************************\n");
}

static void _secsrv_init_resc(struct sp_resc* resc)
{
    strncpy(resc->filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(resc->filepath, "iosm_ps2.srv");
	resc->dma_size = 0;
	resc->heap_size = KB(128); 
	resc->shared_mem_size = KB(128);
	resc->alloc_sp_app_comm = true;
}

static void _secsrv_prepare_params(SL_CFG* sos_cfg, SP_USTRUCT* params) 
{
	// Do nothing
}

/// @brief Allocate resources of IOSM-PS2
/// Requirement: Must run on a different core from SecOS
/// @param sos_cfg 
/// @param out_params 
/// @param out_resc 
static void iosm_ps2_alloc_rescs(SL_CFG* sos_cfg, SP_USTRUCT** out_params, struct sp_resc** out_resc)
{
	int ret = 0;
	SP_USTRUCT* params = NULL;
	struct sp_resc* resc = NULL;
	SP_SIGNATURE_PARAM sp_sig = {.has_1stparty_signature = true, .sp_signature_relative_filepath = "iosm_ps2.sig"};

	// Check: Parameters must be valid
	if(!sos_cfg || !out_params || !out_resc)
		return;

	_print_welcome_message();

	// Allocate memory
	params = (SP_USTRUCT*)app_mm_alloc(sizeof(SP_USTRUCT));
	if(!params)
	{
		SL_INFO("no memory for <params> of IOSM-PS2\n");
		goto err;
	}

	resc = (struct sp_resc*)app_mm_alloc(sizeof(struct sp_resc));
	if(!resc)
	{
		SL_INFO("no memory for <resc> of IOSM-PS2\n");
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
	if(params)
		app_mm_free((vaddr_t)params);
	if(resc)
		app_mm_free((vaddr_t)resc);
	*out_params = NULL;
	*out_resc = NULL;
}

/// @brief Release resources of IOSM-PS2.
/// Requirement: Must run on a different core from SecOS
/// @param params 
/// @param resc 
static void iosm_ps2_dealloc_rescs(SP_USTRUCT* params, struct sp_resc* resc)
{
	int ret = 0;

	// Drain PS2 buffers
	ret = secagent_ps2_drain_buffers();
	if(ret)
		SL_INFO("Failed to drain PS2 buffers\n");

	// Resume PS2 devices
	ret = secagent_resume_ps2_devs();
	if(ret)
		SL_INFO("Failed to resume PS2 devices\n");

	// Finalize the SecApp
	fini_secapp(resc, params);
}

static int iosm_ps2_register(SP_USTRUCT* params)
{
	int ret = 0;
	
	ret = apploader_register_secapp(params);
	if(ret)
	{
		SL_INFO("<apploader_register_secapp> error!\n");
		return -1;
	}

	return 0;
}

static int iosm_ps2_unregister(SP_USTRUCT* params)
{
	apploader_unregister_secapp(params);
	return 0;
}

// [NOTE] This function does not wait for SecOS's response
static int iosm_ps2_run(spid_t sp_id)
{
	int ret = 0;
	
	ret = apploader_run_sp(sp_id);
	if(ret)
	{
		SL_INFO("<apploader_run_sp> error!\n");
		return -1;
	}

	return 0;
}

// [NOTE] This function does not wait for SecOS's response
static int _iosm_ps2_terminate(spid_t sp_id)
{
	int ret = 0;
	
	ret = apploader_soft_terminate_sp(sp_id);
	if(ret)
	{
		SL_INFO("<apploader_soft_terminate_sp> error!\n");
		return -1;
	}

	return 0;
}


static SP_USTRUCT *iosm_ps2_params = NULL;
static struct sp_resc *iosm_ps2_resc = NULL;


int _iosm_ps2_load(SL_CFG *sos_cfg)
{
    iosm_ps2_alloc_rescs(sos_cfg, &iosm_ps2_params, &iosm_ps2_resc);
    if (!iosm_ps2_params || !iosm_ps2_resc)
        return 1;

    return 0;
}

int _iosm_ps2_run(void)
{
    int ret = 0;
    bool initialized = false;

    if (!iosm_ps2_params || !iosm_ps2_resc)
        return 1;

    ret = iosm_ps2_register(iosm_ps2_params);
    if (ret)
    {
        SL_INFO("<iosm_ps2_register> error!\n");
        return 1;
    }

    // Update the record of IOSM state in SOS Loader
    sp_states_add(iosm_ps2_params->sp_id, SP_REGISTERED);

    ret = iosm_ps2_run(iosm_ps2_params->sp_id);
    if (ret)
    {
        SL_INFO("<iosm_ps2_run> error!\n");
        return 1;
    }

    // Wait until the IOSM is running
    initialized = sp_block_wait_init(iosm_ps2_params->sp_id, SECSRV_INIT_TIMEOUT_MS);
    if (!initialized)
    {
        SL_ERR("IOSM-PS2 Initialization error or time out!\n");
        return 1;
    }

    // On success
    SL_INFO("IOSM-PS2 is running!\n");
    return 0;
}

void iosm_ps2_terminate(void)
{
    int ret = 0;
    bool exit_status = false;
    spid_t sp_id;
    SP_STATE state;

    if (!iosm_ps2_params || !iosm_ps2_resc)
        return;

    sp_id = iosm_ps2_params->sp_id;

    // Terminate the running SecProcess and goes to the SP_REGISTERED state
    // [NOTE] If the SecProcess is not running (e.g., SecService encounters initialization errors), then do nothing.
    state = sp_states_get(sp_id);
    if(state == SP_RUNNING)
    {
        ret = _iosm_ps2_terminate(sp_id);
        if (ret)
        {
            SL_ERR("<_iosm_ps2_terminate> error!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Wait until the IOSM is exited
        exit_status = sp_block_wait_exit(sp_id, IOSM_EXIT_TIMEOUT_MS);
        if (!exit_status)
        {
            SL_ERR("IOSM-PS2 exit error or time out!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Update the record of IOSM state in SOS Loader
        sp_states_set(sp_id, SP_REGISTERED);
    }

    // Unregister the SecProcess if it is registered.
    state = sp_states_get(sp_id);
    if ((state == SP_REGISTERED) || (state == SP_EXITED) || (state == SP_INIT_ERR))
    {
        ret = iosm_ps2_unregister(iosm_ps2_params);
        if (ret)
        {
            SL_ERR("<iosm_ps2_unregister> error!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Update the record of IOSM state in SOS Loader
        sp_states_set(sp_id, SP_STATE_INVALID);
    }
}

int _iosm_ps2_unload(void)
{
    spid_t sp_id = INVALID_SP_ID;

    if (!iosm_ps2_params)
        return 1;
    sp_id = iosm_ps2_params->sp_id;

    iosm_ps2_dealloc_rescs(iosm_ps2_params, iosm_ps2_resc);

    // Remove the state record of the IOSM
    sp_states_remove(sp_id);

    return 0;
}