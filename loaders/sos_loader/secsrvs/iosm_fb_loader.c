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

#include <appddk.h>
#include <stdio.h>
#include <string.h>
#include <errno.h> // for errno
#include <pthread.h>
#include <stdlib.h>
#include <sl_headers.h>

#include <iosm_fb_app_intfs.h>
#include <secsrvs/iosm_fb_loader.h>
#include "../include/dev/pci.h"

const union pci_bdf gpu_pci_bdf = {.bus = 0, .dev = 2, .fn = 0};

static OSSTATUS secagent_probe_gpu(union pci_bdf bdf)
{
    bool status = false;
    struct tz_cmd_packet cmd;
    int ret = 0;

    cmd.id = CMD_IOSM_FB_PROBE_GPU;
    cmd.arg1 = bdf.raw;
    status = os_ioctl(&cmd, &ret);
    if (!status)
    {
        SL_INFO("<secagent_probe_gpu> error with code %d!\n", ret);
        return -1;
    }

    return (OSSTATUS)cmd.status;
}

static OSSTATUS secagent_pause_gpu(union pci_bdf bdf)
{
    bool status = false;
    struct tz_cmd_packet cmd;
    int ret = 0;

    cmd.id = CMD_IOSM_FB_PAUSE_GPU;
    cmd.arg1 = bdf.raw;
    status = os_ioctl(&cmd, &ret);
    if (!status)
    {
        SL_INFO("<secagent_pause_gpu> error with code %d!\n", ret);
        return -1;
    }

    return (OSSTATUS)cmd.status;
}

static OSSTATUS secagent_resume_gpu(union pci_bdf bdf)
{
    bool status = false;
    struct tz_cmd_packet cmd;
    int ret = 0;

    cmd.id = CMD_IOSM_FB_RESUME_GPU;
    cmd.arg1 = bdf.raw;
    status = os_ioctl(&cmd, &ret);
    if (!status)
    {
        SL_INFO("<secagent_resume_gpu> error with code %d!\n", ret);
        return -1;
    }

    return (OSSTATUS)cmd.status;
}

static void _print_welcome_message(void)
{
    SL_INFO("******************************\n");
    SL_INFO("Starting the IOSM-FB Service\n");
    SL_INFO("******************************\n");
}

static void _secsrv_init_resc(struct sp_resc *resc)
{
    strncpy(resc->filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(resc->filepath, "iosm_fb.srv");
    resc->dma_size = IOSM_FB_GPU_DMA_MEMORY;
    resc->heap_size = MB(32);
    resc->shared_mem_size = MB(1);
    resc->alloc_sp_app_comm = true;
}

static void _secsrv_prepare_params(SL_CFG *sos_cfg, SP_USTRUCT *params)
{
    struct fb_conf *fb_conf = (struct fb_conf *)params->sp_exec_params_secure;

    fb_conf->fb_bdf = pci_to_bdf(0, 0x2, 0);
    fb_conf->os_enabled_surfaces_num = 1;
    fb_conf->os_pixel_bytes = 4;

    params->sp_exec_params_secure_size = sizeof(struct fb_conf);

    // If using IOSM-FB, notify DesktopMgr to select IOSM-FB
    {
        OSSTATUS sa_status = OS_SUCCESS;
        sa_status = secagent_nsdom_share_data_to_any_ns_process_set(NS_DTM_LOADER_USE_IOSM_FB, DTM_LOADER_USE_IOSM_FB);
        if (SA_FAIL(sa_status))
        {
            APP_ERR("Failed to share desktop_mgr trusted display parameters with other NS processes! SecAgent status:%d\n", sa_status);
            return;
        }
    }
}

/// @brief Allocate resources of IOSM-FB
/// Requirement: Must run on a different core from SecOS
/// @param sos_cfg
/// @param out_params
/// @param out_resc
static void iosm_fb_alloc_rescs(SL_CFG *sos_cfg, SP_USTRUCT **out_params, struct sp_resc **out_resc)
{
    OSSTATUS sa_sts = OS_SUCCESS;
    int ret = 0;
    SP_USTRUCT *params = NULL;
    struct sp_resc *resc = NULL;
    SP_SIGNATURE_PARAM sp_sig = {.has_1stparty_signature = true, .sp_signature_relative_filepath = "iosm_fb.sig"};

    // Check: Parameters must be valid
    if (!sos_cfg || !out_params || !out_resc)
        return;

    _print_welcome_message();

    // Allocate memory
    params = (SP_USTRUCT *)app_mm_alloc(sizeof(SP_USTRUCT));
    if (!params)
    {
        SL_INFO("no memory for <params> of IOSM-FB\n");
        goto err;
    }

    resc = (struct sp_resc *)app_mm_alloc(sizeof(struct sp_resc));
    if (!resc)
    {
        SL_INFO("no memory for <resc> of IOSM-FB\n");
        goto err;
    }

    app_mm_memset(params, 0, sizeof(SP_USTRUCT));
    app_mm_memset(resc, 0, sizeof(struct sp_resc));

    // Init resources
    _secsrv_init_resc(resc);
    _secsrv_prepare_params(sos_cfg, params);
    ret = prepare_secapp(resc, params, &sp_sig);
    if (ret)
        goto err;

    // Probe GPU
    sa_sts = secagent_probe_gpu(gpu_pci_bdf);
    if (SA_FAIL(sa_sts))
    {
        SL_INFO("Failed to probe GPU\n");
        goto err;
    }

    *out_params = params;
    *out_resc = resc;
    return;

err:
    if (params)
        app_mm_free((vaddr_t)params);
    if (resc)
        app_mm_free((vaddr_t)resc);
    *out_params = NULL;
    *out_resc = NULL;
}

/// @brief Release resources of IOSM-FB
/// Requirement: Must run on a different core from SecOS
/// @param params
/// @param resc
static void iosm_fb_dealloc_rescs(SP_USTRUCT *params, struct sp_resc *resc)
{
    // Finalize the SecApp
    fini_secapp(resc, params);
}

static int iosm_fb_register(SP_USTRUCT *params)
{
    int ret = 0;

    ret = apploader_register_secapp(params);
    if (ret)
    {
        SL_INFO("<apploader_register_secapp> error!\n");
        return -1;
    }

    return 0;
}

static int iosm_fb_unregister(SP_USTRUCT *params)
{
    apploader_unregister_secapp(params);
    return 0;
}

// [NOTE] This function does not wait for SecOS's response
static int iosm_fb_run(spid_t sp_id)
{
    int ret = 0;

    ret = apploader_run_sp(sp_id);
    if (ret)
    {
        SL_INFO("<apploader_run_sp> error!\n");
        return -1;
    }

    return 0;
}

// [NOTE] This function does not wait for SecOS's response
static int _iosm_fb_terminate(spid_t sp_id)
{
    int ret = 0;

    ret = apploader_soft_terminate_sp(sp_id);
    if (ret)
    {
        SL_INFO("<apploader_soft_terminate_sp> error!\n");
        return -1;
    }

    return 0;
}

static SP_USTRUCT *iosm_fb_params = NULL;
static struct sp_resc *iosm_fb_resc = NULL;

int _iosm_fb_load(SL_CFG *sos_cfg)
{
    iosm_fb_alloc_rescs(sos_cfg, &iosm_fb_params, &iosm_fb_resc);
    if (!iosm_fb_params || !iosm_fb_resc)
        return 1;

    return 0;
}

int _iosm_fb_run(void)
{
    OSSTATUS sa_sts = OS_SUCCESS;
    OSSTATUS sa_sts2 = OS_SUCCESS;
    int ret = 0;
    bool initialized = false;

    if (!iosm_fb_params || !iosm_fb_resc)
        return 1;

    ret = iosm_fb_register(iosm_fb_params);
    if (ret)
    {
        SL_ERR("<iosm_fb_register> error!\n");
        return 1;
    }

    // Update the record of IOSM state in SOS Loader
    sp_states_add(iosm_fb_params->sp_id, SP_REGISTERED);

    // Pause GPU
    sa_sts = secagent_pause_gpu(gpu_pci_bdf);
    if (SA_FAIL(sa_sts))
    {
        SL_ERR("Failed to pause GPU\n");
        goto pause_gpu_err;
    }

    ret = iosm_fb_run(iosm_fb_params->sp_id);
    if (ret)
    {
        SL_ERR("<iosm_fb_run> error!\n");
        goto iosm_fb_run_err;
    }

    // Wait until the IOSM is running
    initialized = sp_block_wait_init(iosm_fb_params->sp_id, SECSRV_INIT_TIMEOUT_MS);
    if (!initialized)
    {
        SL_ERR("IOSM-FB Initialization error or time out!\n");
        goto sp_block_wait_init_err;
    }

    // On success
    SL_INFO("IOSM-FB is running!\n");
    return 0;

sp_block_wait_init_err:
iosm_fb_run_err:
    sa_sts2 = secagent_resume_gpu(gpu_pci_bdf);
    if (SA_FAIL(sa_sts2))
        SL_ERR("Failed to resume GPU after IOSM-FB run error!\n");
pause_gpu_err:
    return 1;
}

void iosm_fb_terminate(void)
{
    OSSTATUS sa_sts = OS_SUCCESS;
    int ret = 0;
    bool exit_status = false;
    spid_t sp_id;
    SP_STATE state;

    if (!iosm_fb_params || !iosm_fb_resc)
        return;

    sp_id = iosm_fb_params->sp_id;

    // Terminate the running SecProcess and goes to the SP_REGISTERED state
    // [NOTE] If the SecProcess is not running (e.g., SecService encounters initialization errors), then do nothing.
    state = sp_states_get(sp_id);
    if (state == SP_RUNNING)
    {
        ret = _iosm_fb_terminate(sp_id);
        if (ret)
        {
            SL_ERR("<_iosm_fb_terminate> error!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Wait until the IOSM is exited
        exit_status = sp_block_wait_exit(sp_id, IOSM_EXIT_TIMEOUT_MS);
        if (!exit_status)
        {
            SL_ERR("IOSM-FB exit error or time out!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Resume GPU
        sa_sts = secagent_resume_gpu(gpu_pci_bdf);
        if (SA_FAIL(sa_sts))
            SL_ERR("Failed to resume GPU\n");

        // Update the record of IOSM state in SOS Loader
        sp_states_set(sp_id, SP_REGISTERED);
    }

    // Unregister the SecProcess if it is registered.
    state = sp_states_get(sp_id);
    if ((state == SP_REGISTERED) || (state == SP_EXITED) || (state == SP_INIT_ERR))
    {
        ret = iosm_fb_unregister(iosm_fb_params);
        if (ret)
        {
            SL_ERR("<iosm_fb_unregister> error!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Update the record of IOSM state in SOS Loader
        sp_states_set(sp_id, SP_STATE_INVALID);
    }
}

int _iosm_fb_unload(void)
{
    spid_t sp_id = INVALID_SP_ID;

    if (!iosm_fb_params)
        return 1;
    sp_id = iosm_fb_params->sp_id;

    iosm_fb_dealloc_rescs(iosm_fb_params, iosm_fb_resc);

    // Remove the state record of the IOSM
    sp_states_remove(sp_id);

    return 0;
}