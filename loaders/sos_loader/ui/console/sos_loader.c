// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sl_headers.h>

void sl_main_clean_on_exception(void)
{
	// Do nothing
}

int main()
{
	SL_STATUS status = SL_SUCCESS;
	int ret = 0;

    // Init log sub-system.
    status = sl_log_init();
    if(SL_FAIL(status))
    {
        printf("Initialize log error! status:%lu\n", status);
        ret = -1;
        goto out;
    }
    // SOSLoader can use print macros (e.g., SL_DEBUG, SL_INFO, SL_ERR) from now on.

    // Run scripts to prepare GreenBox
    status = sl_run_prepare_script();
    if(SL_FAIL(status))
    {
        printf("Run GreenBox prepare script error! status:%lu\n", status);
        ret = -1;
        goto out;
    }
    msleep(1000);

	status = sl_init();
	if(SL_FAIL(status))
	{
		SL_ERR("SOSLoader init error! status:%lu\n", status);
		ret = -1;
        goto out;
	}

	press_continue("Press to continue");

	status = sl_load_and_run_secos_and_secsrvs();
	if(SL_FAIL(status))
	{
		SL_ERR("SOSLoader runs SecOS and SecServices error! status:%lu\n", status);
		if(sl_state_get() != SECOS_UNLOADED && status != SL_EINVAL)
		{
			ret = -1;
			goto sos_fini_from_running;
		}

		ret = -1;
		goto sos_fini_from_not_running;
	}

	// Wait for user commands:
	press_continue("Press to exit SecServices and SecOS");


sos_fini_from_running:
	status = sl_terminate_and_unload_secos_and_secsrvs();
	if(SL_FAIL(status))
	{
		SL_ERR("SOSLoader terminates SecOS error! status:%lu\n", status);
        ret = -1;
	}

sos_fini_from_not_running:
	status = sl_fini();
	if(SL_FAIL(status))
	{
		SL_ERR("SOSLoader finalize error! status:%lu\n", status);
        ret = -1;
	}

out:
    sl_log_fini();

	return ret;
}






// int main()
// {
// 	// // gpu_debug_without_start_secos();

// 	// app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_1);
// 	// secagent_test1();


//     SL_STATUS status = SUCCESS;
//     char xhci_hc_pci_addr_str[USB_HC_BDF_STR_LENGTH_MAX + 1] = {0};
//     uint32_t usb_bus_id = 1;

//     status = usb_hc_get_pci_address((uint32_t)usb_bus_id, xhci_hc_pci_addr_str);
//     if(SL_FAIL(status))
//     {
//         SL_ERR("SOSLoader: Failed to get the PCI address of the USB host controller (bus:%u)! status:%lu\n", usb_bus_id, status);
//         return -1;
//     }
//     SL_INFO("PCI address of the USB host controller (bus:%u): %s\n", usb_bus_id, xhci_hc_pci_addr_str);
// }









// bool app_sa_notify_init(void);
// void app_sa_notify_fini(void);
// void secagent_test3(void);

// int main()
// {
// 	app_sa_notify_init();

// 	// press_continue("Press to exit SecApp");
// 	secagent_test3();
// 	msleep(1000);
// 	secagent_test3();
// 	msleep(1000);

// 	app_sa_notify_fini();
// }



// int main()
// {
// 	SL_STATUS status = SL_SUCCESS;
// 	int ret = 0;

// 	status = sl_init();
// 	if(SL_FAIL(status))
// 	{
// 		SL_ERR("SOSLoader init error! status:%lu\n", status);
// 		return -1;
// 	}

// 	press_continue("Press to continue");

// 	status = sl_load_and_run_secos_and_secsrvs();
// 	if(SL_FAIL(status))
// 	{
// 		SL_ERR("SOSLoader runs SecOS and SecServices error! status:%lu\n", status);
// 		if(status != SL_EINVAL)
// 		{
// 			ret = -1;
// 			goto main_out;
// 		}
// 		return -1;
// 	}

// 	// Wait for user commands:
// 	press_continue("Press to exit SecServices and SecOS");

// main_out:
// 	// status = sl_terminate_and_unload_secos_and_secsrvs();
// 	// if(SL_FAIL(status))
// 	// {
// 	// 	ret = -1;
// 	// 	SL_ERR("SOSLoader terminates SecOS error! status:%lu\n", status);
// 	// }

// 	status = sl_fini();
// 	if(SL_FAIL(status))
// 	{
// 		ret = -1;
// 		SL_ERR("SOSLoader finalize error! status:%lu\n", status);
// 	}

// 	return ret;
// }



// int main()
// {
//     SL_STATUS status = SL_SUCCESS;

//     status = usb_hc_reset_by_vendor_product_str("VEN_8086&DEV_A2AF");
//     printf("status:%lu\n", status);

//     return 0;
// }


// void sl_minimal_load_and_run(void);
// int main()
// {
// 	sl_minimal_load_and_run();

// 	return 0;
// }






// void secagent_test3(void);

// int main()
// {
// 	secagent_test3();

// 	return 0;
// }




// static SL_CFG sos_cfg;
// int main()
// {
//     SL_STATUS status = SL_SUCCESS;

// 	// Init log sub-system.
//     status = sl_log_init();
//     if(SL_FAIL(status))
//     {
//         printf("Initialize log error! status:%lu\n", status);
//         return -1;
//     }
//     // SOSLoader can use print macros (e.g., SL_DEBUG, SL_INFO, SL_ERR) from now on.

//     press_continue("Press to continue");

//     // Read Greenbox configurations
//     status = sl_cfg_read(&sos_cfg);
//     if (SL_FAIL(status))
//     {
//         SL_ERR("Read GreenBox configuration error! status:%lu\n", status);
//         return -1;
//     }

//     // SL_INFO("PCI address of the USB host controller (bus:%u): %s\n", usb_bus_id, out_cfg->usb3_info.xhci_hc_pci_addr_str);

//     sl_log_fini();
// 	return 0;
// }
