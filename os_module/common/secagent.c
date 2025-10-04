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

#include <sa_headers.h>

static struct os_funcs os_funcs;
struct os_funcs *os = &os_funcs;
static bool is_sa_running = true;

void secagent_mark_running(void)
{
	is_sa_running = true;
}

void secagent_mark_not_running(void)
{
	is_sa_running = false;
}

OSSTATUS secagent_init(void)
{
	OSSTATUS status = OS_SUCCESS;
	
	// Initialize required OS functions
	os_init();

	// Test the presence of Secbase
	if(!os->secbase_test_presence())
	{
		// Use <PRINT_ERR_FORCE_OS> instead of <DRV_ERR>. Otherwise, the SecAgent may crash due to sending hypercalls
		// to the non-exist mHV
		PRINT_ERR_FORCE_OS("SecBase is not running!\n");
		status = E_CHKERR;
		goto test_presence_err;
	}
	
	// Provision resources needed by SecOS
	DRV_INFO("Loading SecAgent\n");
	__x86_os_cpu_lapic_mapping_init();
	// status = sos_comm_resc_provision();
	// if(SA_FAIL(status))
	// {
	// 	DRV_ERR("Provision Memory of SecOS-OS Communication Channel Error!\n");
	// 	goto mm_resc_provision_err;
	// }

	// // Setup SecOS-OS communication of the SecAgent's side
	// status = comm_sos_init(vk_manifest.vk_os_rx_chanctl_start_vaddr, 
	// 	vk_manifest.vk_os_rx_chanbuf_start_vaddr, 
	// 	vk_manifest.vk_os_tx_chanctl_start_vaddr, 
	// 	vk_manifest.vk_os_tx_chanbuf_start_vaddr
	// );
	// if(SA_FAIL(status))
	// {
	// 	DRV_ERR("Setup SecOS-OS Communication Channel Error!\n"); 
	// 	goto sos_init_err;
	// }

    // Init SecAgent-App communication
	//// Init asynchronous event notifications
	os->os_app_notif_subsystem_init();
	//// Init event caching subsystem
    app_notif_mgr_init();

	// OSSTATUS win_sos_os_intr_probe_and_register(void);
	// win_sos_os_intr_probe_and_register();

    // Init utils functions for the NS domain 
    status = nsdom_utils_init();
    if(SA_FAIL(status))
    {
        DRV_ERR("Init NS domain utils functions error!\n"); 
        goto nsdom_utils_init_err;
    }
	
	// On success
	secagent_mark_running();
	DRV_INFO("SecAgent Initialization Succeed!\n");
	return OS_SUCCESS;


nsdom_utils_init_err:
// sos_init_err:
// 	sos_comm_resc_return();
// mm_resc_provision_err:
test_presence_err:
	os_fini();
	secagent_mark_not_running();
	return status;
}

void secagent_fini(void)
{
	DRV_INFO("SecAgent is exiting.\n");
	secagent_mark_not_running();

    // Finalize utils functions for the NS domain 
    nsdom_utils_fini();

	// void win_sos_os_intr_unregister(void);
	// win_sos_os_intr_unregister();

#ifdef WINNT
	// [TODO][Issue 104] Due to Issue 104, SecAgent only invokes IoDisconnectInterrupt during DriverUnload
	// os->sos_os_intr_unregister();
#endif

    // Finalize SecAgent-App communication
    app_notif_mgr_fini();
	//// Fini asynchronous event notifications
	os->os_app_notif_subsystem_fini();

	__x86_os_cpu_lapic_mapping_fini();

	// Finalize required OS functions
	os_fini();
}

OSSTATUS do_CMD_SECAGENT_PRESENCE_REQ(struct tz_cmd_packet *cmd_packet)
{
	UNUSED(cmd_packet);
	
	if(!is_sa_running)
		return E_SA_NOT_RUNNING;

	return OS_SUCCESS;
}








// // [Superymk]
// OSSTATUS secagent_init(void)
// {
// 	OSSTATUS status = OS_SUCCESS;
	
// 	// Initialize required OS functions
// 	os_init();

// 	// Provision resources needed by SecOS
// 	DRV_INFO("Loading SecAgent\n");


// 	// Step 1. Provision the interrupt vector for SecOS-OS and SecOS-SecBase channels
// 	status = os->sos_os_intr_probe_and_register();
// 	if(SA_FAIL(status))
// 	{
// 		DRV_ERR("<secagent_init> Probe and Register SecOS-OS Interrupt Error! status:%lu\n", status);
// 		status = E_INTERNAL_ERROR;
// 		goto intr_reg_err;
// 	}

// 	return OS_SUCCESS;


// intr_reg_err:
// 	os->sos_os_intr_unregister();
// 	return status;
// }
// // [Superymk] End
