// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <appddk.h>
#include <sl_headers.h>
#include <elf.h>

#include "../hash/hash.h"

static struct sl_sos_sect_addrs secos_addrs = {0};
static struct mem_alignment_info _sos_signature_start_info;
static struct mem_alignment_info _sos_mempool_start_info;
static struct mem_alignment_info _sos_img_info;

static uint32_t cmd = 0;

/// @brief The CPU that executes <sl_secos_load>
static uint32_t _sos_load_thread_affinity = 0;

static int ld_sos_alloc_resource(struct sl_sos_sect_addrs *secos_addrs)
{
    vaddr_t sos_mem_start_va = INVALID_VADDR;
    vaddr_t sos_mem_end_va = 0;

    // Register SecOS heap memory
    sos_mem_start_va = app_mm_alloc_and_lock_4Kpages_for_sdom(SOS_HEAP_SIZE >> PAGE_SHIFT_4K, &_sos_mempool_start_info);
    if (sos_mem_start_va == INVALID_VADDR)
        return -1;

    sos_mem_end_va = sos_mem_start_va + SOS_HEAP_SIZE;
    secos_addrs->sos_mempool_start = sos_mem_start_va;
    secos_addrs->sos_mempool_end = sos_mem_end_va;
    secos_addrs->sos_mempool_dest_gpa = K_VIRT_TO_PHYS(SOS_HEAP_BASE);

    // [TODO][Ticket97] OS kernel may enable large pages by default and cause XMHF crash, we need to think a workaround
    return 0;
}

static int ld_sos_signature(struct sl_sos_sect_addrs *secos_addrs)
{
    bool ret = false;
    char secos_signature_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};
    vaddr_t sos_signature_start_va = INVALID_VADDR;
    vaddr_t sos_signature_end_va = 0;
    SECOS_SIGNATURE *sos_signature = NULL;

    strncpy(secos_signature_file_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(secos_signature_file_path, "verifiable-kernel.sig");

    // Register SecOS signature memory
    sos_signature_start_va = app_mm_alloc_and_lock_4Kpages_for_sdom(SOS_SIGNATURE_PAGE_SIZE >> PAGE_SHIFT_4K, &_sos_signature_start_info);
    if (sos_signature_start_va == INVALID_VADDR)
        return -1;

    sos_signature_end_va = sos_signature_start_va + SOS_SIGNATURE_PAGE_SIZE;
    secos_addrs->sos_signature_start = sos_signature_start_va;
    secos_addrs->sos_signature_end = sos_signature_end_va;
    secos_addrs->sos_signature_dest_gpa = K_VIRT_TO_PHYS(SOS_SIGNATURE_PAGE_BASE);

    // Load SecOS signature
    sos_signature = (SECOS_SIGNATURE *)sos_signature_start_va;
    ret = file_read(secos_signature_file_path, (void *)sos_signature->signature, 0, SECOS_SIGNATURE_LENGTH);
    if (!ret)
    {
        SL_ERR("Read SecOS signature error!\n");
        return -1;
    }

    // [TODO][Ticket97] OS kernel may enable large pages by default and cause XMHF crash, we need to think a workaround
    return 0;
}

/// @brief Load the SecOS image into memory, return its vaddr and the memory size.
/// @param secos_addrs 
/// @return 
static SL_STATUS ld_sos_load_image(struct sl_sos_sect_addrs *secos_addrs)
{
    SL_STATUS status = SL_SUCCESS;
    bool ret = false;
    struct elfhdr elf = {0};
    struct proghdr *ph = NULL, *eph = NULL;
    size_t total_size = 0;
    vaddr_t img_vaddr = INVALID_VADDR;
    uint32_t img_offset = 0;
    char secos_img_file_path[FS_MAX_PATH_BUFFER_LEN] = {0};

    strncpy(secos_img_file_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(secos_img_file_path, "verifiable-kernel.img");

    ret = file_read(secos_img_file_path, &elf, 0, sizeof(struct elfhdr));
    if (!ret)
    {
        SL_ERR("Read SecOS image error!\n");
        return SL_EINVAL;
    }

    // Is this an ELF executable?
    if (elf.magic != ELF_MAGIC)
        return SL_EINVAL;

    // Calculate the total size of all the sections
    ph = (struct proghdr *)((vaddr_t)&elf + elf.phoff);
    eph = ph + elf.phnum;
    for (; ph < eph; ph++)
    {
        if (!ph->paddr)
            continue;

        total_size += ph->memsz;
    }

    total_size = SIZE_ROUNDUP_4K(total_size);
    img_vaddr = app_mm_alloc_and_lock_4Kpages_for_sdom(total_size >> PAGE_SHIFT_4K, &_sos_img_info);
    if (img_vaddr == INVALID_VADDR)
    {
        return SL_EINVAL;
    }
    memset((void *)img_vaddr, 0, total_size);

    // load all the sections into memory
    ph = (struct proghdr *)((vaddr_t)&elf + elf.phoff);
    eph = ph + elf.phnum;
    for (; ph < eph; ph++)
    {
        vaddr_t img_pos = (vaddr_t)img_vaddr + img_offset;

        if (!ph->paddr)
            continue;

        ret = file_read(secos_img_file_path, (void *)img_pos, ph->off, ph->filesz);
        if (!ret)
        {
            SL_ERR("Read SecOS image section error!\n");
            status = SL_EINVAL;
            goto err;
        }

        if (ph->memsz > ph->filesz)
            app_mm_memset((void *)(img_pos + ph->filesz), 0, ph->memsz - ph->filesz);

        img_offset += ph->memsz;
    }

    // Register the memory region of SecOS image
    secos_addrs->sos_image_start = img_vaddr;
    secos_addrs->sos_image_end = img_vaddr + total_size;
    secos_addrs->sos_image_dest_gpa = K_VIRT_TO_PHYS(SOS_CODE_BASE);

    return SL_SUCCESS;

err:
    if (img_vaddr)
        app_mm_free_and_unlock_4Kpages_for_sdom(&_sos_img_info);
    return status;
}

static void release_resource(struct sl_sos_sect_addrs *secos_addrs)
{
    // Release the memory allocated for the SecOS
    if(secos_addrs->sos_image_start != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&_sos_img_info);
        secos_addrs->sos_image_start = INVALID_VADDR;
    }

    if (secos_addrs->sos_mempool_start != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&_sos_mempool_start_info);
        secos_addrs->sos_mempool_start = INVALID_VADDR;
    }

    if (secos_addrs->sos_signature_start != INVALID_VADDR)
    {
        app_mm_free_and_unlock_4Kpages_for_sdom(&_sos_signature_start_info);
        secos_addrs->sos_signature_start = INVALID_VADDR;
    }
}

/// @brief Pass input and output parameters between <thread_run_secos> and its caller.
struct secos_run_params
{
    struct sl_sos_sect_addrs *sos_addrs; // Input parameter <sos_addrs>

    SL_STATUS status; // Return status <status>
};

static struct secos_run_params sos_run_params = {0};

static void _sync_sos_run_params_status_set(struct secos_run_params* params, SL_STATUS new_status)
{
    (void)__sync_lock_test_and_set(&params->status, new_status);
}

// The main thread to run SecOS
static void *thread_run_secos(void *data)
{
    int status = 0;
    struct secos_run_params *sos_params = (struct secos_run_params *)data;
    struct sl_sos_sect_addrs *sos_addrs = sos_params->sos_addrs;

    // Run on Core 1
    app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_1);

    // [TODO][Issue 116] SOSLoader may not register and run SecOS on CPU 1 if calling <kick_all_other_processes_from_core>
    // status2 = kick_all_other_processes_from_core(SP_LOADER_CPU_AFFINITY_CORE_1);
    // if(!status2)
    // 	SL_ERR("Failed to un-schedule other processes from SecOS's CPU core!\n");

    // [TODO][Issue 115] SECSRV-INIT on linux will hang if enabling <thread_set_high_priority>
    // status2 = thread_set_high_priority();
    // if(!status2)
    // 	SL_ERR("Failed to raise priority of the SecOS execution thread!\n");

    // Accept notifications sent from SecAgent
    app_sa_notify_init();

    // Register the SecOS
    status = secagent_register_secos_prepare();
    if (status)
    {
        SL_ERR("SecOS registration preparation error!\n");
        _sync_sos_run_params_status_set(sos_params, SL_E_SOS_PROVISION);
        goto out;
    }

    status = secagent_register_secos(sos_addrs);
    if (status)
    {
        _sync_sos_run_params_status_set(sos_params, SL_E_SOS_REGISTER);
        goto out;
    }
    sl_state_transit(SECOS_LOADED, SECOS_REGISTERED);

    mdelay(100);

    // Boot the SecOS
    status = secagent_run_secos();
    if (status)
    {
        _sync_sos_run_params_status_set(sos_params, SL_E_SOS_RUN);
        goto out;
    }

    // secagent_trigger_all_intrs(sos_status->intr);
    SL_INFO("SecOS boot status:0x%08X\n", status);

    // Run the SecOS
    sl_state_transit(SECOS_REGISTERED, SECOS_RUNNING);

    while (cmd == RUN)
    {
        bool secagent_success = true;

        // Get SecOS status
        status = secagent_get_secos_status(&secagent_success);
        if (!secagent_success)
        {
            SL_ERR("Failed to get SecOS's status!\n");
            break;
        }

        // secagent_trigger_all_intrs(sos_status->intr);

        if (status == SECOS_INFO_PREEMPTED || status == SECOS_INFO_RETURN_FROM_PREEMPTION)
        {
            // Resume SecOS execution, if SecOS is not exited
            // [NOTE] Stay in red OS/Apps for 1ms so red OS on this core can response cross-core requests (e.g., system
            // calls that need to be handled by all CPUs).
            // Why usleep but not mdelay? mdelay may sleep longer (~20ms ish). SecOS does not expect to yield CPU for
            // that long.
            // usleep(10000);

            sos_resume();

            // udelay(1000);
            // sleep_ms(10);
        }
        else
        {
            SL_INFO("SecOS status2:0x%08X\n", status);
            _sync_sos_run_params_status_set(sos_params, SL_E_SOS_RUN);
            cmd = STOP;
        }

        // sos_status->intr[0] = 0;
        // sos_status->intr[1] = 0;
        // sos_status->intr[2] = 0;
        // sos_status->intr[3] = 0;
        // sos_status->intr[4] = 0;
        // sos_status->intr[5] = 0;
        // sos_status->intr[6] = 0;
        // sos_status->intr[7] = 0xFFFFFFFF;
        // secagent_trigger_all_intrs(sos_status->intr);

        // mdelay(10);
    }

    sl_state_transit(SECOS_RUNNING, SECOS_REGISTERED);

out:
    return NULL;
}

#define BLOCK_WAIT_MSLEEP_PER_ROUND (10)

/// @brief Returns true iff SecOS is booted
/// @param timeout_ms 
/// @return 
static bool _block_wait_secos_booted(uint32_t timeout_ms)
{
    VKSTATUS status = SUCCESS;
    bool secagent_success = true;
    bool secos_booted = false;
    volatile uint32_t timeout_round = timeout_ms / BLOCK_WAIT_MSLEEP_PER_ROUND;

    do
    {
        status = secagent_get_secos_status(&secagent_success);

        // It is possible that secagent_success == false, because the other thread has not booted SecOS yet.
        // Thus, we do not print errors of <secagent_get_secos_status> and just wait SecOS to boot.
        if (secagent_success)
        {
            if ((status == SECOS_INFO_PREEMPTED) || (status == SECOS_INFO_RETURN_FROM_PREEMPTION))
            {
                secos_booted = true;
                break;
            }
        }

        // Errors happen in <thread_run_secos>, so SecOS is not booted.
        if (sos_run_params.status != SL_SUCCESS)
        {
            secos_booted = false;
            break;
        }

        udelay(BLOCK_WAIT_MSLEEP_PER_ROUND * 1000);
        timeout_round--;
    } while (timeout_round != 0);

    // Ensure SecOS is fully initialized
    if (secos_booted)
    {
        udelay(1000);
        return true;
    }

    return false; // SecOS is not booted due to errors or timeout.
}

static bool _block_wait_sos_terminated(uint32_t timeout_ms)
{
    VKSTATUS status = SUCCESS;
    bool secagent_success = true;
    bool secos_terminated = false;
    volatile uint32_t timeout_round = timeout_ms / BLOCK_WAIT_MSLEEP_PER_ROUND;

    do
    {
        status = secagent_get_secos_status(&secagent_success);

        // It is possible that secagent_success == false, because the other thread has not booted SecOS yet.
        // Thus, we do not print errors of <secagent_get_secos_status> and just wait SecOS to boot.
        if (secagent_success)
        {
            if ((status == SECOS_INFO_EXITED))
            {
                secos_terminated = true;
                break;
            }
        }

        udelay(BLOCK_WAIT_MSLEEP_PER_ROUND * 1000);
        timeout_round--;
    } while (timeout_round != 0);

    // Ensure SecOS is fully terminated
    if (secos_terminated)
    {
        usleep(1000);
        return true;
    }

    return false; // Timeout return, so return error
}




/********* Public Functions *********/
void ld_sos_run_control_set(uint32_t new_cmd)
{
    // Update <current_state> and ensure cache coherence (memory propagation)
    (void)__sync_lock_test_and_set(&cmd, new_cmd);
}

SL_STATUS sl_secos_load(SL_CFG* sos_cfg, uint32_t cpu_affinity)
{
    SL_STATUS status = SL_SUCCESS;
    int ret = 0;
    
    _sos_load_thread_affinity = cpu_affinity;
    app_set_cpu_affinity(_sos_load_thread_affinity);

    // Load SecOS image
    status = ld_sos_load_image(&secos_addrs);
    if (SL_FAIL(status))
    {
        SL_ERR("Load SecOS image error! status:%lu\n", status);
        goto load_img_err;
    }

    if (secos_addrs.sos_image_start == INVALID_VADDR)
    {
        SL_ERR("SecOS image load error\n");
        status = SL_EINVAL;
        goto load_img_err;
    }

    // Allocate resources to SecOS
    ret = ld_sos_alloc_resource(&secos_addrs);
    if (ret)
    {
        SL_ERR("Provision SecOS resource error\n");
        status = SL_EINVAL;
        goto alloc_sos_resource_err;
    }

    // Load SecOS signature
    ret = ld_sos_signature(&secos_addrs);
    if (ret)
    {
        SL_ERR("Load SecOS signature error\n");
        status = SL_EINVAL;
        goto ld_signature_err;
    }

    // On success
    sl_state_transit(SECOS_UNLOADED, SECOS_LOADED);
    return SL_SUCCESS;


ld_signature_err:
alloc_sos_resource_err:
load_img_err:
    sl_secos_unload();
    (void)sl_tasks_run_after_unload_secos();
    return status;
}

void sl_secos_unload(void)
{
    // Check: SecOS must be loaded
    if(sl_state_get() != SECOS_LOADED)
        return;

    sl_state_transit(SECOS_LOADED, SECOS_UNLOADED);

    SL_INFO("Unloading SecOS...\n");
    app_set_cpu_affinity(_sos_load_thread_affinity);

    release_resource(&secos_addrs);

    _sos_load_thread_affinity = 0;

    SL_INFO("SecOS is unloaded!\n");
}

SL_STATUS sl_secos_register_and_run(void)
{
    SL_STATUS status = SL_SUCCESS;
    bool sos_booted = true;
    pthread_t secos_thread;

    // Check: SecOS must be loaded
    if(sl_state_get() != SECOS_LOADED)
        return SL_EINVAL;

    // Register and run the SecOS
    ld_sos_run_control_set(RUN);
    sos_run_params.sos_addrs = &secos_addrs;
    //// Clear <sos_run_params.status>, so other threads of the loader can track updates of <sos_params->status> due to
    //// errors.
    _sync_sos_run_params_status_set(&sos_run_params, SL_SUCCESS);
    pthread_create(&secos_thread, NULL, thread_run_secos, &sos_run_params);

    // Wait until SecOS is booted
    app_set_cpu_affinity(_sos_load_thread_affinity);

    sos_booted = _block_wait_secos_booted(5000); 
    if (!sos_booted)
    {
        if (sos_run_params.status != SL_SUCCESS)
        {
            SL_ERR("Boot SecOS failed!\n");
            status = sos_run_params.status;
            goto sos_register_and_boot_err;
        }
        else
        {
            SL_ERR("Boot SecOS timeout!\n");
            status = SL_E_SOS_RUN;
            goto sos_register_and_boot_err;
        }
    }

    // On success
    return SL_SUCCESS;

sos_register_and_boot_err:
    if(sl_state_get() == SECOS_RUNNING)
    {
        ld_secos_terminate();
        ld_secos_unregister();
    }
    else if(sl_state_get() == SECOS_REGISTERED)
    {
        ld_secos_unregister();
    }

    if(sl_state_get() != SECOS_LOADED)
    {
        SL_ERR("Boot SecOS error: SecOS loader should return to SECOS_LOADED on error!\n");
    }

    return status;
}

SL_STATUS ld_secos_terminate(void)
{
    SL_STATE sl_state = sl_state_get();

    // Check: SecOS must be registered or running
    if((sl_state != SECOS_REGISTERED) && (sl_state != SECOS_RUNNING))
        return SL_EINVAL;

    sl_state_transit(SL_STATE_ANY, SECOS_REGISTERED);

    // Must run on a different CPU
    app_set_cpu_affinity(_sos_load_thread_affinity);

    if(sl_state == SECOS_RUNNING)
    {
        bool secos_terminated = false;

        // Tell the SecOS to exit
        SL_INFO("Terminating SecOS...\n");
        secagent_terminate_secos();

        // Wait SecOS is terminated
        secos_terminated = _block_wait_sos_terminated(2000);
        if (secos_terminated)
        {
            SL_INFO("Terminated SecOS\n");
        }
        else
        {
            SL_ERR("Terminating SecOS timeout!\n");
            // No need to return error here. The function will always terminate the thread <thread_run_secos> in the 
            // next.
        }
    }

    // Terminate the thread <thread_run_secos>
    ld_sos_run_control_set(STOP);
    // mdelay(100);
    
    // On success
    return SL_SUCCESS;
}

SL_STATUS ld_secos_unregister(void)
{
    SL_STATE sl_state = sl_state_get();
    
    // Check: SecOS must be registered or running
    if((sl_state != SECOS_REGISTERED))
        return SL_EINVAL;

    sl_state_transit(SECOS_REGISTERED, SECOS_LOADED);

    // Run on the core dedicated for SecOS
    app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_1);

    // SecOS finishes execution, now we can unregister it
    SL_INFO("\n");
    SL_INFO("**********************\n");
    SL_INFO("*****Unregister SecOS*****\n");
    SL_INFO("**********************\n");
    SL_INFO("\n");

    secagent_unregister_secos();

    // Disable notifications sent from SecAgent
    app_sa_notify_fini();
    
    // On success
    return SL_SUCCESS;
}