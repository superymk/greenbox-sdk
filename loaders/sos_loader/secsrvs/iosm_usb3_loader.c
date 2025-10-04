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

#include <iosm_usb3_app_intfs.h>
#include <secsrvs/iosm_usb3_loader.h>
#include "../include/dev/pci.h"

static void _usbpdev_addr_copy(USBPDEV_ADDR *to, USBPDEV_ADDR *from)
{
    to->v.low = from->v.low;
    to->v.high = from->v.high;
}

// static uint32_t secagent_probe_xhci(union pci_bdf xhci_bdf)
// {
// 	OSSTATUS sa_status = OS_SUCCESS;
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;

// 	cmd.id = CMD_IOSM_USB3_SHARED_XHCI_PROBE;
//     cmd.arg1 = xhci_bdf.raw;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		SL_INFO("<secagent_probe_xhci> error with code %d!\n", ret);
// 		return 1;
// 	}

// 	sa_status = cmd.status;
// 	if(SA_FAIL(sa_status))
// 	{
// 		SL_INFO("<secagent_probe_xhci> SecAgent error: %d!\n", sa_status);
// 		return 1;
// 	}

// 	return 0;
// }

// static uint32_t secagent_unprobe_xhci(void)
// {
// 	OSSTATUS sa_status = OS_SUCCESS;
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;

// 	cmd.id = CMD_IOSM_USB3_SHARED_XHCI_UNPROBE;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		SL_INFO("<secagent_unprobe_xhci> error with code %d!\n", ret);
// 		return 1;
// 	}

// 	sa_status = cmd.status;
// 	if(SA_FAIL(sa_status))
// 	{
// 		SL_INFO("<secagent_unprobe_xhci> SecAgent error: %d!\n", sa_status);
// 		return 1;
// 	}

// 	return 0;
// }

// static uint32_t secagent_pause_xhci(void)
// {
// 	OSSTATUS sa_status = OS_SUCCESS;
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;

// 	cmd.id = CMD_IOSM_USB3_SHARED_XHCI_PAUSE;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		SL_INFO("<secagent_pause_xhci> error with code %d!\n", ret);
// 		return 1;
// 	}

// 	sa_status = cmd.status;
// 	if(SA_FAIL(sa_status))
// 	{
// 		SL_INFO("<secagent_pause_xhci> SecAgent error: %d!\n", sa_status);
// 		return 1;
// 	}

// 	return 0;
// }

// static uint32_t secagent_resume_xhci(void)
// {
// 	OSSTATUS sa_status = OS_SUCCESS;
// 	bool status = false;
// 	struct tz_cmd_packet cmd;
// 	int ret = 0;

// 	cmd.id = CMD_IOSM_USB3_SHARED_XHCI_RESUME;
// 	status = os_ioctl(&cmd, &ret);
// 	if(!status)
// 	{
// 		SL_INFO("<secagent_resume_xhci> error with code %d!\n", ret);
// 		return 1;
// 	}

// 	sa_status = cmd.status;
// 	if(SA_FAIL(sa_status))
// 	{
// 		SL_INFO("<secagent_resume_xhci> SecAgent error: %d!\n", sa_status);
// 		return 1;
// 	}

// 	return 0;
// }

// static uint32_t secagent_ping_xhci_interrupts(void)
// {
// 	uint32_t status = 0, status2 = 0;
// 	uint32_t intr_irq = 0;
// 	uint32_t i = 0;
// 	uint32_t num_cpus = app_cpus_get_procs_num();

// 	status2 = secagent_x86_os_irq_get_intr_vector(20, &intr_irq);
// 	if(status2)
// 	{
// 		SL_ERR("Get the interrupt vector of IRQ 20 error! status:%lu\n", status2);
// 		status = 1;
// 	}

// 	for(i = 0; i < num_cpus; i++)
// 	{
// 		app_set_cpu_affinity(i);
// 		(void)secagent_os_trigger_intr(intr_irq);
// 	}

// 	return status;
// }

static void _print_welcome_message(void)
{
    SL_INFO("******************************\n");
    SL_INFO("Starting the IOSM-USB3 Service\n");
    SL_INFO("******************************\n");
}

static int _secsrv_init_resc(struct sp_resc *resc)
{
    strncpy(resc->filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(resc->filepath, "iosm_usb3.srv");
    resc->dma_size = MB(4); // MB(1) + DMA Space for each USBPDev driver
    resc->heap_size = MB(1);
    resc->shared_mem_size = MB(1);
    resc->alloc_sp_app_comm = true;

    // On success
    return 0;
}

static int _secsrv_prepare_params(SL_CFG *sos_cfg, SP_USTRUCT *params)
{
    struct usb_conf *usb_conf = (struct usb_conf *)params->sp_exec_params_secure;
    uint8_t hid_dev1_usb_addr = sos_cfg->usb3_info.hid_dev1_addr;
    uint8_t hid_dev1_hub_addr = sos_cfg->usb3_info.hid_dev1_hub_addr;
    uint8_t hid_dev1_hub_port = sos_cfg->usb3_info.hid_dev1_hub_port;
    uint16_t hid_dev1_bus_id = sos_cfg->usb3_info.hid_dev1_bus_id;
    uint8_t hid_dev2_usb_addr = sos_cfg->usb3_info.hid_dev2_addr;
    uint8_t hid_dev2_hub_addr = sos_cfg->usb3_info.hid_dev2_hub_addr;
    uint8_t hid_dev2_hub_port = sos_cfg->usb3_info.hid_dev2_hub_port;
    uint16_t hid_dev2_bus_id = sos_cfg->usb3_info.hid_dev2_bus_id;

    usb_conf->xhci_bdf = os_usb_pci_addr_string_to_bdf(sos_cfg->usb3_info.xhci_hc_pci_addr_str);
    usb_conf->num_usb_nodes = 0;
    usb_conf->os_hc_roothub_usb_addr = USB_HC_ROOTHUB_USB_ADDRESS;

    // Fill the info of USB nodes
    //// Root hub
    {
        struct usb_node *target_node_conf = &usb_conf->usb_nodes[usb_conf->num_usb_nodes];

        USBPDEV_ADDR rh_addr = {
            .bus_id = hid_dev1_bus_id,
            .dev_usbaddr = hid_dev1_hub_addr,
            .hub_port = NONEXIST_UPSTREAM_HUB_PORT,    // No upper level hub
            .hub_usbaddr = NONEXIST_UPSTREAM_HUB_ADDR, // No upper level hub
            .reserved2 = 0x7FF,
            .reserved1 = 0xFFFF};

        _usbpdev_addr_copy(&target_node_conf->node_addr, &rh_addr);
        // target_node_conf->dev_vendor_id = 0;
        // target_node_conf->dev_product_id = 0;

        target_node_conf->tp_usbpdev = false;
        target_node_conf->is_root_hub = true;
        target_node_conf->is_hub = true;

        usb_conf->num_usb_nodes++;
    }

    // //// Rate matching hub
    // {
    // 	struct usb_node* target_node_conf = &usb_conf->usb_nodes[usb_conf->num_usb_nodes];

    // 	USBPDEV_ADDR rmh_addr = {
    // 		.bus_id = XHCI_BUS_NO,
    // 		.dev_usbaddr = 2,
    // 		.hub_port = 1, // Port 1 of Root Hub
    // 		.hub_usbaddr = 1, // USB Address of Root Hub
    // 		.reserved2 = 0x7FF,
    // 		.reserved1 = 0xFFFF
    // 	};

    // 	_usbpdev_addr_copy(&target_node_conf->node_addr, &rmh_addr);
    // 	target_node_conf->tp_usbpdev = false;
    // 	target_node_conf->is_root_hub = false;
    // 	target_node_conf->is_hub = true;

    // 	usb_conf->num_usb_nodes++;
    // }

    // USBPDev 1
    if (hid_dev1_usb_addr != 0)
    {
        struct usb_node *target_node_conf = &usb_conf->usb_nodes[usb_conf->num_usb_nodes];

        USBPDEV_ADDR addr = {
            .bus_id = hid_dev1_bus_id,
            .dev_usbaddr = hid_dev1_usb_addr,
            .hub_port = hid_dev1_hub_port,
            .hub_usbaddr = hid_dev1_hub_addr,
            .reserved2 = 0x7FF,
            .reserved1 = 0xFFFF};

        _usbpdev_addr_copy(&target_node_conf->node_addr, &addr);
        // target_node_conf->dev_vendor_id = sos_cfg->usb3_info.hid_dev1_vendor_id;
        // target_node_conf->dev_product_id = sos_cfg->usb3_info.hid_dev1_product_id;

        target_node_conf->tp_usbpdev = true;
        target_node_conf->is_root_hub = false;
        target_node_conf->is_hub = false;

        usb_conf->num_usb_nodes++;
    }

    // USBPDev 2
    if (hid_dev2_usb_addr != 0)
    {
        struct usb_node *target_node_conf = &usb_conf->usb_nodes[usb_conf->num_usb_nodes];

        USBPDEV_ADDR addr = {
            .bus_id = hid_dev2_bus_id,
            .dev_usbaddr = hid_dev2_usb_addr,
            .hub_port = hid_dev2_hub_port,
            .hub_usbaddr = hid_dev2_hub_addr,
            .reserved2 = 0x7FF,
            .reserved1 = 0xFFFF};

        _usbpdev_addr_copy(&target_node_conf->node_addr, &addr);
        // target_node_conf->dev_vendor_id = sos_cfg->usb3_info.hid_dev2_vendor_id;
        // target_node_conf->dev_product_id = sos_cfg->usb3_info.hid_dev2_product_id;

        target_node_conf->tp_usbpdev = true;
        target_node_conf->is_root_hub = false;
        target_node_conf->is_hub = false;

        usb_conf->num_usb_nodes++;
    }

    params->sp_exec_params_secure_size = sizeof(struct usb_conf);

    //[TODO][Issue 23] Trusted SecProcesses must present valid signatures

    return 0;
}

/// @brief Allocate resources of IOSM-USB3
/// Requirement: Must run on a different core from SecOS
/// @param sos_cfg
/// @param out_params
/// @param out_resc
static void iosm_usb3_alloc_rescs(SL_CFG *sos_cfg, SP_USTRUCT **out_params, struct sp_resc **out_resc)
{
    int ret = 0;
    SP_USTRUCT *params = NULL;
    struct sp_resc *resc = NULL;
    SP_SIGNATURE_PARAM sp_sig = {.has_1stparty_signature = true, .sp_signature_relative_filepath = "iosm_usb3.sig"};

    // Check: Parameters must be valid
    if (!sos_cfg || !out_params || !out_resc)
        return;

    _print_welcome_message();

    // Allocate memory
    params = (SP_USTRUCT *)app_mm_alloc(sizeof(SP_USTRUCT));
    if (!params)
    {
        SL_ERR("no memory for <params> of IOSM-USB3\n");
        goto err;
    }

    resc = (struct sp_resc *)app_mm_alloc(sizeof(struct sp_resc));
    if (!resc)
    {
        SL_ERR("no memory for <resc> of IOSM-USB3\n");
        goto err;
    }

    app_mm_memset(params, 0, sizeof(SP_USTRUCT));
    app_mm_memset(resc, 0, sizeof(struct sp_resc));

    // Init resources
    ret = _secsrv_init_resc(resc);
    if (ret)
        goto err;

    ret = _secsrv_prepare_params(sos_cfg, params);
    if (ret)
        goto err;

    ret = prepare_secapp(resc, params, &sp_sig);
    if (ret)
        goto err;

    // // Probe XHCI
    // {
    //     struct usb_conf* usb_conf = (struct usb_conf*)params->sp_exec_params_secure;

    //     ret = secagent_probe_xhci(usb_conf->xhci_bdf);
    //     if(ret)
    //     {
    //         SL_ERR("Failed to probe XHCI\n");
    //         goto err;
    //     }
    // }

    // // Pause XHCI
    // ret = secagent_pause_xhci();
    // if(ret)
    // {
    // 	SL_ERR("Failed to pause XHCI\n");
    // 	goto err;
    // }

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

/// @brief Release resources of IOSM-USB3
/// Requirement: Must run on a different core from SecOS
/// @param params
/// @param resc
static void iosm_usb3_dealloc_rescs(SP_USTRUCT *params, struct sp_resc *resc)
{
    // int ret = 0;

    // // Resume XHCI
    // ret = secagent_resume_xhci();
    // if(ret)
    // 	SL_ERR("Failed to resume XHCI\n");

    // ret = secagent_ping_xhci_interrupts();
    // if(ret)
    // 	SL_ERR("Failed to trigger XHCI interrupts\n");

    // Finalize the SecApp
    fini_secapp(resc, params);
}

// //! Re-registering IOSM-USB3 does not access XHCI's PCI config space prior to calling this function. This allows users to
// //! restart IOSM-USB3 without restarting IOSM-PCIE, which exclusively owns the I/O ports to access all PCI config spaces.
// //! After IOSM-PCIE quits (i.e., all I/O separation kernels quit), one should call this function this free infos stored
// //! in SecAgent.
// void iosm_usb3_unprobe_resc_info(void)
// {
// 	int ret = 0;

// 	// Unprobe XHCI
// 	ret = secagent_unprobe_xhci();
// 	if(ret)
// 		SL_ERR("Failed to unprobe XHCI\n");
// }

static int iosm_usb3_register(SP_USTRUCT *params)
{
    int ret = 0;

    ret = apploader_register_secapp(params);
    if (ret)
    {
        SL_ERR("<apploader_register_secapp> error!\n");
        return -1;
    }

    return 0;
}

static int iosm_usb3_unregister(SP_USTRUCT *params)
{
    apploader_unregister_secapp(params);
    return 0;
}

// [NOTE] This function does not wait for SecOS's response
static int iosm_usb3_run(spid_t sp_id)
{
    int ret = 0;

    ret = apploader_run_sp(sp_id);
    if (ret)
    {
        SL_ERR("<apploader_run_sp> error!\n");
        return -1;
    }

    return 0;
}

// [NOTE] This function does not wait for SecOS's response
static int _iosm_usb3_terminate(spid_t sp_id)
{
    int ret = 0;

    ret = apploader_soft_terminate_sp(sp_id);
    if (ret)
    {
        SL_ERR("<apploader_soft_terminate_sp> error!\n");
        return -1;
    }

    return 0;
}

static SP_USTRUCT *iosm_usb3_params = NULL;
static struct sp_resc *iosm_usb3_resc = NULL;

int _iosm_usb3_load(SL_CFG *sos_cfg)
{
    iosm_usb3_alloc_rescs(sos_cfg, &iosm_usb3_params, &iosm_usb3_resc);
    if (!iosm_usb3_params || !iosm_usb3_resc)
        return 1;

    return 0;
}

int _iosm_usb3_run(void)
{
    int ret = 0;
    bool initialized = false;

    if (!iosm_usb3_params || !iosm_usb3_resc)
        return 1;

    ret = iosm_usb3_register(iosm_usb3_params);
    if (ret)
    {
        SL_ERR("<iosm_usb3_register> error!\n");
        return 1;
    }

    // Update the record of IOSM state in SOS Loader
    sp_states_add(iosm_usb3_params->sp_id, SP_REGISTERED);

    ret = iosm_usb3_run(iosm_usb3_params->sp_id);
    if (ret)
    {
        SL_ERR("<iosm_usb3_run> error!\n");
        return 1;
    }

    // Wait until the IOSM is running
    initialized = sp_block_wait_init(iosm_usb3_params->sp_id, SECSRV_INIT_TIMEOUT_MS);
    if (!initialized)
    {
        SL_ERR("IOSM-USB3 Initialization error or time out!\n");
        return 1;
    }

    // On success
    SL_INFO("IOSM-USB3 is running!\n");
    return 0;
}

void iosm_usb3_terminate(void)
{
    int ret = 0;
    bool exit_status = false;
    spid_t sp_id;
    SP_STATE state;

    if (!iosm_usb3_params || !iosm_usb3_resc)
        return;

    sp_id = iosm_usb3_params->sp_id;

    // Terminate the running SecProcess and goes to the SP_REGISTERED state
    // [NOTE] If the SecProcess is not running (e.g., SecService encounters initialization errors), then do nothing.
    state = sp_states_get(sp_id);
    if (state == SP_RUNNING)
    {
        ret = _iosm_usb3_terminate(sp_id);
        if (ret)
        {
            SL_ERR("<_iosm_usb3_terminate> error!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Wait until the IOSM is exited
        exit_status = sp_block_wait_exit(sp_id, IOSM_EXIT_TIMEOUT_MS);
        if (!exit_status)
        {
            SL_ERR("IOSM-USB3 exit error or time out!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Update the record of IOSM state in SOS Loader
        sp_states_set(sp_id, SP_REGISTERED);
    }

    // Unregister the SecProcess if it is registered.
    state = sp_states_get(sp_id);
    if ((state == SP_REGISTERED) || (state == SP_EXITED) || (state == SP_INIT_ERR))
    {
        ret = iosm_usb3_unregister(iosm_usb3_params);
        if (ret)
        {
            SL_ERR("<iosm_usb3_unregister> error!\n");
            // [NOTE] We do not return early on error, so the rest of this function can proceed.
        }

        // Update the record of IOSM state in SOS Loader
        sp_states_set(sp_id, SP_STATE_INVALID);
    }
}

int _iosm_usb3_unload(void)
{
    spid_t sp_id = INVALID_SP_ID;

    if (!iosm_usb3_params)
        return 1;
    sp_id = iosm_usb3_params->sp_id;

    iosm_usb3_dealloc_rescs(iosm_usb3_params, iosm_usb3_resc);

    // Remove the state record of the IOSM
    sp_states_remove(sp_id);

    return 0;
}