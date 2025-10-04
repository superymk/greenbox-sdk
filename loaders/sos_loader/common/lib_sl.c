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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sl_headers.h>

#include <elf.h>
#include <secsrvs/sl_secsrvs.h>

#include <dev/usb.h>

#include "../hash/hash.h"
#include "sl_sb_tpm.h"

static SL_CFG sos_cfg;

__attribute__((unused)) static void _x86_probe_lapic_id(void)
{
    uint32_t i = 0;
    uint32_t num_cpus = app_cpus_get_procs_num();

    for (i = 0; i < num_cpus; i++)
    {
        app_set_cpu_affinity(i);
        secagent_x86_init_collect_lapic_id_percpu();
    }
}

__attribute__((unused)) static void _secagent_init_secbase(void)
{
    uint32_t i = 0;
    uint32_t num_cpus = app_cpus_get_procs_num();

    for (i = 0; i < num_cpus; i++)
    {
        app_set_cpu_affinity(i);
        secagent_init_secbase_ondemand_percpu();
    }
}

__attribute__((unused)) static void _secagent_fini_secbase(void)
{
    uint32_t i = 0;
    uint32_t num_cpus = app_cpus_get_procs_num();

    for (i = 0; i < num_cpus; i++)
    {
        app_set_cpu_affinity(i);
        secagent_fini_secbase_ondemand_percpu();
    }
}

static void sl_install_exception_handler(void)
{
    sp_loader_install_exception_handler();
}

static SL_STATUS _greenbox_run_prepare_script(void)
{
    SL_STATUS status = SUCCESS;
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char param1[6] = {0};
    char param2[FS_MAX_PATH_BUFFER_LEN] = {0}; // Param 2 - absolute path of SecAgent

    // Prepare the parameters
    sprintf(param1, "%d", SP_LOADER_CPU_AFFINITY_CORE_1);
    strncpy(param2, apploader_get_exe_directory(), MAX_PATH_LEN);
    
#if defined(__WINDOWS__)
    strcat(param2, "SecAgent.sys");

    const char *exe_filename = "gb_prepare.bat";
    const union sha_digest _file_gb_prepare_sha256 = {
        .sha2_256_digest = {
            0x0c, 0x0e, 0xea, 0x3d, 0xe3, 0x1f, 0xbc, 0x7d,
            0x8d, 0x08, 0x89, 0x47, 0xcf, 0x51, 0x0c, 0xcb,
            0x84, 0xa5, 0xcb, 0xcf, 0xb4, 0x73, 0xd5, 0x7d,
            0x47, 0xff, 0xc1, 0x18, 0xbc, 0x87, 0x96, 0x30
        }
    };
#else
    strcat(param2, "secagent.ko");

    const char *exe_filename = "gb_prepare.sh";
    const union sha_digest _file_gb_prepare_sha256 = {
        .sha2_256_digest = {
            0x10, 0x0b, 0x3c, 0x3b, 0xc0, 0x43, 0x0a, 0x58,
            0x24, 0x7c, 0xe2, 0x63, 0xe7, 0xf4, 0x9c, 0x77,
            0xe8, 0xcf, 0x45, 0xca, 0x56, 0x9e, 0x2e, 0x38,
            0xc4, 0xc9, 0x42, 0xa0, 0xfc, 0x19, 0x47, 0x78
        }
    };
    
#endif // __WINDOWS__
    int ret = 0;

    // Run the script to prepare GreenBox.
    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);
    ret = os_execute_program_wait_exit(true, exe_filepath, &_file_gb_prepare_sha256, exe_filename, param1, param2, NULL);
    if (ret)
    {
        status = SL_EINTERNAL;
    }
    else
    {
        status = SL_SUCCESS;
    }

    // Return status
    return status;
}


/************ Public Functions **********/
SL_STATUS sl_run_prepare_script(void)
{
    return _greenbox_run_prepare_script();
}

SL_STATUS sl_log_init(void)
{
    SL_STATUS status = SL_SUCCESS;
    char log_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    // Init the log subsystem. 
    status = greenbox_get_root_path(log_filepath);
    if (SL_FAIL(status))
        return SL_EINVAL;
    strcat(log_filepath, SOSLOADER_LOG_ROOT_RELATIVE_FILEPATH);

    status = sp_loader_log_init(log_filepath);
    if (SL_FAIL(status))
    {
        return status;
    }

    return SL_SUCCESS;
}

void sl_log_fini(void)
{
    sp_loader_log_fini();
}

SL_STATUS sl_init(void)
{
    bool secagent_presence = false;

    // Detect the presence of SecAgent
    secagent_presence = secagent_test_presence();
    if (!secagent_presence)
    {
        SL_ERR("SecAgent is not loaded. Exit!\n");
        return SL_EINVAL;
    }

    sp_loader_os_init();
    sl_state_init();

    // Init thru SecAgent
    _secagent_init_secbase();
    _x86_probe_lapic_id();

    sl_state_transit(SL_STATE_INVALID, SECOS_UNLOADED);

    // Step 0. Init - SOS Loader
    sp_states_init();

    sl_install_exception_handler();

    // [NOTE] <sl_init> does not set CPU affinity, because this function always runs before SecOS execution.

    return SL_SUCCESS;
}

SL_STATUS sl_fini(void)
{
    app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_2);

    // Fini thru SecAgent
    _secagent_fini_secbase();

    // Fini - SOS Loader
    sp_states_fini();

    sl_state_fini();
    sp_loader_os_fini();

    return SL_SUCCESS;
}

SL_STATUS sl_load_and_run_secos_and_secsrvs(void)
{
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;
    uint8_t os_already_muted = (uint8_t)false;

    SL_INFO("Starting SecOS and SecServices...\n");

    // Read Greenbox configurations
    status = sl_cfg_read(&sos_cfg);
    if (SL_FAIL(status))
    {
        SL_ERR("Read GreenBox configuration error! status:%lu\n", status);
        return status;
    }

    //// Store the number of components to boot 
	{
		OSSTATUS sa_status = OS_SUCCESS;
		sa_status = secagent_nsdom_share_data_to_any_ns_process_set(NS_GB_COMPONENTS_NUM, sos_cfg.num_components_to_boot);
		if(SA_FAIL(sa_status))
		{
			SL_ERR("Failed to store number of components to boot! SecAgent status:%d\n", sa_status);
			status = SL_EINTERNAL;
            goto sl_store_num_components_err;
		}
	}

    status = sl_secos_load(&sos_cfg, SP_LOADER_CPU_AFFINITY_CORE_2);
    if (SL_FAIL(status))
    {
        SL_ERR("SecOS load error! status:%lu\n", status);
        goto sl_secos_load_err;
    }

    // Load SecServices
    ret = secsrvs_load(&sos_cfg, SP_LOADER_CPU_AFFINITY_CORE_2);
    if (ret)
    {
        SL_ERR("Load SecServices error!\n");
        status = SL_EINTERNAL;
        goto secsrvs_load_err;
    }

    status = ld_secsrvs_init_before_secos_run(&sos_cfg);
    if (SL_FAIL(status))
    {
        SL_ERR("Init SecServices before SecOS error! status:%lu\n", status);
        goto ld_secsrvs_init_before_secos_run_err;
    }

    // Mute the untrusted OS because the sound glitches when booting the SecOS and SecServices.
    ret = os_sound_mute(sl_load_and_run_secos_and_secsrvs, &os_already_muted);
    if (ret)
    {
        SL_WARN("Init SecOS and SecServices: Untrusted OS mute error! status:%d\n", ret);
    }

    // Run SecOS
    status = sl_secos_register_and_run();
    if (SL_FAIL(status))
    {
        SL_ERR("SecOS register and run error! status:%lu\n", status);
        goto sl_secos_register_and_run_err;
    }

    ret = secsrvs_run(&sos_cfg);
    if (ret)
    {
        SL_ERR("Run SecServices error!\n");
        status = SL_EINTERNAL;
        goto sl_secsrvs_run_err;
    }

    // Unmute the untrusted OS after the SecOS and SecServices are booted.
    ret = os_sound_unmute(sl_load_and_run_secos_and_secsrvs, os_already_muted);
    if (ret)
    {
        SL_WARN("Init SecOS and SecServices: Untrusted OS unmute error! status:%d\n", ret);
    }

    // On success
    SL_INFO("SecOS and SecServices are running.\n");
    return SL_SUCCESS;

sl_secsrvs_run_err:
    // Terminate all SecServices. <secsrvs_run> does not kill running SecServices if failing to boot a SecService.
    secsrvs_terminate(&sos_cfg);

    // Terminate SecOS
    ld_secos_terminate();
    ld_secos_unregister();

sl_secos_register_and_run_err:
    // Unmute the untrusted OS on SecOS/SecServices run errors.
    (void)os_sound_unmute(sl_load_and_run_secos_and_secsrvs, os_already_muted);
    ld_secsrvs_fini_after_secos_terminate(&sos_cfg);
ld_secsrvs_init_before_secos_run_err:
    secsrvs_unload(&sos_cfg);
secsrvs_load_err:
    sl_secos_unload();
    (void)sl_tasks_run_after_unload_secos();
sl_secos_load_err:
sl_store_num_components_err:
    return status;
}

SL_STATUS sl_terminate_and_unload_secos_and_secsrvs(void)
{
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;
    uint8_t os_already_muted = (uint8_t)false;

    SL_INFO("Terminating SecOS and SecServices...\n");

    // Mute the untrusted OS because the sound glitches when terminating the SecOS and SecServices.
    ret = os_sound_mute(sl_terminate_and_unload_secos_and_secsrvs, &os_already_muted);
    if (ret)
    {
        SL_WARN("Finalize SecOS and SecServices: Untrusted OS mute error! status:%d\n", ret);
    }

    // Terminate SecServices
    secsrvs_terminate(&sos_cfg);

    // Terminate SecOS
    ld_secos_terminate();
    ld_secos_unregister();

    // Unmute the untrusted OS after terminating the SecOS and SecServices.
    //// [TODO][Issue 163] DesktopMgr may terminate and have not unmuted the OS before this functions runs. In this case,
    //// <os_already_muted> of this function is true, which is incorrect.
    //// For now, we force <os_already_muted> to be false to solve this issue temporially.
    os_already_muted = false;

    ret = os_sound_unmute(sl_terminate_and_unload_secos_and_secsrvs, os_already_muted);
    if (ret)
    {
        SL_WARN("Finalize SecOS and SecServices: Untrusted OS unmute error! status:%d\n", ret);
    }
    
    ld_secsrvs_fini_after_secos_terminate(&sos_cfg);

    // Release resources of SecServices
    secsrvs_unload(&sos_cfg);

    sl_secos_unload();

    // Run the tasks that are required right after SecOS success termination and unload.
    status = sl_tasks_run_after_unload_secos();
    if(SL_FAIL(status))
    {
        SL_ERR("Finalize SecOS and SecServices: Failed to run tasks required after SecOS unload! status:%d\n", ret);
    }

    // On success
    SL_INFO("SecOS and SecServices are terminated.\n");
    return SL_SUCCESS;
}

SL_CFG *sl_get_sos_cfg(void)
{
    return &sos_cfg;
}

#define DMESG_BUF_SIZE (MB(1))

SL_STATUS sl_dump_dmesg(void)
{
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;
    char dmesg_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};
    size_t dmesg_size = 0;
    uint8_t* dmesg_buf = NULL;
    struct mem_alignment_info dmesg_buf_info;

    status = greenbox_get_root_path(dmesg_file_path);
    if(SL_FAIL(status))
    {
        SL_ERR("Failed to locate the root folder of GreenBox!\n");
        status = SL_EINTERNAL;
        goto out;
    }

    strcat(dmesg_file_path, SOSLOADER_DMESG_LOG_ROOT_RELATIVE_FILEPATH);

    // lock <dmesg_buf>
    dmesg_buf = (uint8_t*)app_mm_alloc_and_lock_4Kpages_for_sdom(DMESG_BUF_SIZE >> PAGE_SHIFT_4K, &dmesg_buf_info);
    if(!dmesg_buf)
    {
        SL_ERR("Failed to allocate dmesg buffer!\n");
        status = SL_ENOMEM;
        goto out;
    }

    // Read dmesg from SecBase
    ret = hypercall_read_dmesg(dmesg_buf, &dmesg_size);
    if (ret)
    {
        SL_ERR("Failed to read dmesg from SecBase!\n");
        status = SL_EINTERNAL;
        goto out;
    }

    // Write dmesg to file
    ret = fs_write_file(dmesg_file_path, dmesg_buf, dmesg_size);
    if (ret)
    {
        SL_ERR("Failed to write dmesg to dmesg.log!\n");
        status = SL_EINTERNAL;
        goto out;
    }

    // On success
    status = SL_SUCCESS;

out:
    if(dmesg_buf)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&dmesg_buf_info);
    }
    return status;
}

/********* SecProcess *********/
SL_STATUS sl_secprocesses_soft_terminate(spid_t sp_id)
{
    int ret = apploader_soft_terminate_sp(sp_id);

    if (ret)
        return SL_EINTERNAL;

    return SL_SUCCESS;
}

SL_STATUS sl_secprocesses_get_ids_and_names(struct sp_info_id_name_resp *resp_buf)
{
    uint32_t ret = 0;

    ret = secagent_sps_get_ids_and_names(resp_buf);

    if (ret)
        return SL_EINTERNAL;

    return SL_SUCCESS;
}

/********* SecBase TPM Functions *********/
SL_STATUS sl_secbase_tpm_create_keys(void)
{
    return _do_sl_secbase_tpm_create_keys();
}

SL_STATUS sl_secbase_tpm_get_quote(struct tpm_quote_nonce* nonce, uint32_t pcr_mhv_img, uint32_t pcr_mhv_boot_state)
{
    return _do_sl_secbase_tpm_get_quote(nonce, pcr_mhv_img, pcr_mhv_boot_state);
}

SL_STATUS sl_secbase_tpm_get_pcrs(uint32_t pcr_mhv_img, uint32_t pcr_mhv_boot_state)
{
    return _do_sl_secbase_tpm_get_pcrs(pcr_mhv_img, pcr_mhv_boot_state);
}


/********* Untrusted OS TPM Functions *********/
SL_STATUS sl_os_efi_tpm2_event_logs_get_trace(uint32_t pcr_mhv_img)
{
    return _do_sl_os_efi_tpm2_event_logs_get_trace(pcr_mhv_img);
}

/********* SecAgent Event Callback *********/
#define SP_INIT_STATUS_SUCCESS (SUCCESS)

void on_secagent_notify(void)
{
    OSSTATUS status = OS_SUCCESS;
    struct sa_app_notif_info siginfo;

    // Poll signal's detailed info until there is no more info from SecAgent
    while (status != E_SIG_NO_MORE)
    {
        status = secagent_poll_notification_info(&siginfo);
        if (status != OS_SUCCESS && status != E_SIG_NO_MORE)
        {
            SL_ERR("Failed to poll signal from SecAgent! status:%d\n", status);
            break;
        }

        // Update IOSM state according to signal info reported by SecAgent
        if (status == OS_SUCCESS)
        {
            if (siginfo.info1 == SOS_SIGNAL_INFO1_SP_INITIALIZED)
            {
                // Exit related, so update the corresponding IOSM's state to be running.
                spid_t sp_id = siginfo.sp_id;

                if (siginfo.info2 == SP_INIT_STATUS_SUCCESS)
                {
                    // SecProcess initialization succeeds
                    if (!sp_states_set(sp_id, SP_RUNNING))
                    {
                        SL_WARN("Record SecProcess %u's running state failed\n", sp_id);
                    }
                }
                else
                {
                    // SecProcess initialization fails
                    if (!sp_states_set(sp_id, SP_INIT_ERR))
                    {
                        SL_WARN("Record SecProcess %u's initialization error state failed\n", sp_id);
                    }
                }
            }
            else if (siginfo.info1 == SOS_SIGNAL_INFO1_SP_NORMAL_EXIT ||
                     siginfo.info1 == SOS_SIGNAL_INFO1_SP_FORCE_EXIT ||
                     siginfo.info1 == SOS_SIGNAL_INFO1_SP_EXCEPTION_EXIT)
            {
                // Exit related, so update the corresponding IOSM's state to be exited.
                spid_t sp_id = siginfo.sp_id;
                if (!sp_states_set(sp_id, SP_EXITED))
                {
                    SL_ERR("Warning: sp_states_set(sp_id, SP_EXITED) failed\n");
                }
            }
        }
    }
}

void sl_on_exception(void)
{
    // Force terminate the SecOS
    sl_terminate_and_unload_secos_and_secsrvs();

    // [TODO][Issue 101] The current implementation can only force terminating SecOS and SecServices with Ctrl+C once. The machine hangs if running SecOS and Services and force terminating them again.
    // How to reproduce the issue: (1) Enable only the INIT, (2) Add "usleep(10000);" before "sos_resume();" in <thread_run_secos>, and run the sos_loader (console).
    (void)sl_fini();

    sl_main_clean_on_exception();
}

/************ Debug Functions **********/
SL_STATUS sl_sos_minimal_load_and_run(void)
{
    SL_STATUS status = SL_SUCCESS;

    status = sl_secos_load(&sos_cfg, SP_LOADER_CPU_AFFINITY_CORE_2);
    if (SL_FAIL(status))
    {
        SL_ERR("SecOS load error! status:%lu\n", status);
        goto sl_secos_load_err;
    }

    // Run SecOS
    status = sl_secos_register_and_run();
    if (SL_FAIL(status))
    {
        SL_ERR("SecOS register and run error! status:%lu\n", status);
        goto sl_secos_register_and_run_err;
    }

    // On success
    return SL_SUCCESS;

// sl_secsrvs_run_err:
    ld_secos_terminate();
    ld_secos_unregister();

sl_secos_register_and_run_err:
    sl_secos_unload();
sl_secos_load_err:
    return status;
}


// // #include <debugapi.h>

// // [Superymk]
// void secagent_test2(ulong_t arg1);

// extern SL_STATUS ld_sos_load_image(struct sl_sos_sect_addrs *secos_addrs);

// void thread_minimal_run_secos(struct sl_sos_sect_addrs *p_secos_addrs)
// {
//     int status = 0;

//     // Run on Core 1
//     app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_1);

//     // Register the SecOS
//     status = secagent_register_secos_prepare();
//     if (status)
//     {
//         SL_ERR("SecOS registration preparation error!\n");
//         goto out;
//     }

//     // secagent_test2((vaddr_t)p_secos_addrs);

//     status = secagent_register_secos(p_secos_addrs);
//     if (status)
//         goto out;

//     // Boot the SecOS
//     SL_INFO("Booting SecOS\n");
//     status = secagent_run_secos();
//     SL_INFO("SecOS boot status:0x%08X\n", status);
//     // mfence();
//     // while(1){}

//     // SecOS finishes execution, now we can unregister it
//     SL_INFO("\n");
//     SL_INFO("**********************\n");
//     SL_INFO("*****Unregister SecOS*****\n");
//     SL_INFO("**********************\n");
//     SL_INFO("\n");

//     secagent_unregister_secos();

//     release_resource(p_secos_addrs);

//     (void)status;
//     if (false)
//         goto out;

//     SL_INFO("SecOS is terminated!\n");

// out:
//     return;
// }

// void sl_minimal_load_and_run(void)
// {
//     int ret = 0;

//     // Init SecAgent
//     _secagent_init_secbase();
//     _x86_probe_lapic_id();

//     // // Step 1. Load SecOS image
//     // ret = ld_sos_load_image(&secos_addrs);

//     // sos_img_vaddr = secos_addrs.sos_image_start;
//     // if (sos_img_vaddr == INVALID_VADDR)
//     // {
//     //     SL_ERR("SecOS image load error\n");
//     //     return;
//     // }

//     // // Step 2. Allocate resources to SecOS
//     // ret = ld_sos_alloc_resource(&secos_addrs);
//     // if (ret)
//     // {
//     //     SL_ERR("[Error] Provision SecOS resource error\n");
//     //     return;
//     // }

//     sl_secos_load(NULL, SP_LOADER_CPU_AFFINITY_CORE_2);

//     thread_minimal_run_secos(&secos_addrs);
// }
// // [Superymk] End




// SL_STATUS sl_load_and_run_secos_and_secsrvs_test(void)
// {
//     SL_STATUS status = SL_SUCCESS;

//     // Read Greenbox configurations
//     status = sl_cfg_read(&sos_cfg);
//     if (SL_FAIL(status))
//     {
//         SL_ERR("Read GreenBox configuration error! status:%lu\n", status);
//         return status;
//     }

    

//     status = ld_secsrvs_init_before_secos_run(&sos_cfg);
//     if (SL_FAIL(status))
//     {
//         SL_ERR("Init SecServices before SecOS error! status:%lu\n", status);
//         return status;
//     }

//     press_continue("<ld_secsrvs_init_before_secos_run>");

//     ld_secsrvs_fini_after_secos_terminate(&sos_cfg);

//     // On success
//     return SL_SUCCESS;
// }









