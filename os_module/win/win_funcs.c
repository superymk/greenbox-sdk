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

#include "win_cpu.h"
#include "win_mm.h"
#include "win_intr.h"
#include "win_event_notify.h"
#include "win_mm_sdom.h"

#include <sa_headers.h>

bool cpu_support_clflush = false;

static void win_cfgs_init(void)
{
	int edx;
	int cpuInfo[4];
	
	__cpuid(cpuInfo, 1);
	edx = cpuInfo[3];
	
	// Support CLFLUSH?
	if(edx & CPUID_FLAG_CLFL)
		cpu_support_clflush = true;
}

static void win_cfgs_fini(void)
{
	cpu_support_clflush = false;
}

// Refer to Windows Internals Part 2, 7th edition, Table 8-4 "Predefined interrupt vectors" 
#define WIN10_APC_INTR_VECTOR	(0x1F)
#define WIN10_DPC_INTR_VECTOR	(0x2F)
#define WIN10_IPI_INTR_VECTOR	(0xE1)

static void win_sos_boot_params_init(struct sos_boot_params* boot_params)
{
	boot_params->sos_compat_conf = SOS_COMPATIBILITY_WINDOWS;

	// Tell SecOS to forward APC interrupts, DPC interrupts, and IPI interrupts to the red OS immediately.
	boot_params->os_intrs_immediate[0] = WIN10_APC_INTR_VECTOR;
	boot_params->os_intrs_immediate[1] = WIN10_DPC_INTR_VECTOR;
	boot_params->os_intrs_immediate[2] = WIN10_IPI_INTR_VECTOR;
	boot_params->os_intrs_immediate_num = 3;
}




/********* Public Functions *********/
void os_init(void)
{
	os->sos_boot_params_init = win_sos_boot_params_init;

	// cpu functions
	os->processor_id = win_read_processor_id;
	os->read_msr = win_read_msr;
	os->write_msr = win_write_msr;
	os->atomic_increase = win_atomic_increase;
    os->atomic_decrease = win_atomic_decrease;
    os->spinlock_init = win_spin_lock_init;
	os->spinlock_lock = win_spin_lock;
    os->spinlock_trylock = win_spin_trylock;
	os->spinlock_unlock = win_spin_unlock;
	os->udelay = win_udelay;
	os->inb = win_inb;
	os->inw = win_inw;
	os->inl = win_inl;
	os->outb = win_outb;
	os->outw = win_outw;
	os->outl = win_outl;

	// process functions
    os->get_current_pid = win_get_current_pid;
	
	// memory functions
	os->mm_malloc_zeroed = win_mm_malloc_zeroed;
	os->mm_free = win_mm_free;
	os->mm_fence = win_mm_fence;
	os->mm_vaddr_to_paddr = win_mm_vaddr_to_paddr;
	os->mm_alloc_pages = win_mm_alloc_pages;
	os->mm_alloc_physical_contiguous_pages = win_mm_get_physical_contiguous_pages;
	os->mm_alloc_physical_contiguous_pages_nocache = win_mm_alloc_physical_contiguous_pages_nocache;
	os->mm_free_pages = win_mm_free_pages;
	os->mm_free_physical_contiguous_pages = win_mm_free_physical_contiguous_pages;
	os->mm_ioremap_nocache = win_ioremap_nocache;
	os->mm_iounmap = win_iounmap;
    os->mm_alloc_and_lock_pages_for_sdom = win_mm_alloc_and_lock_pages_for_sdom;
    os->mm_free_and_unlock_pages_for_sdom_by_kvaddr = win_mm_free_and_unlock_pages_for_sdom_by_kvaddr;
    os->mm_free_and_unlock_pages_for_sdom_by_uvaddr = win_mm_free_and_unlock_pages_for_sdom_by_uvaddr;
	os->mm_memset = win_memset;
	os->mm_memcpy = win_memcpy;
	os->mm_check_current_process_page_permission = win_mm_check_current_process_page_permission;
	os->copy_to_user = win_copy_to_user;
	os->copy_from_user = win_copy_from_user;
	os->wbinvd = win_wbinvd;
	os->cache_flush_range = win_cache_flush_range;

	/********* For Asynchronous Notifications from SecAgent to Untrusted Applications (e.g., SecProcess Loaders) *********/
	os->os_app_notif_subsystem_init = win_os_app_notif_subsystem_init;
	os->os_app_notif_subsystem_fini = win_os_app_notif_subsystem_fini;
	os->os_app_notif_register = win_os_app_notif_register;
	os->os_app_notif_unregister = win_os_app_notif_unregister;
	os->os_notify_app = win_signal_app;
	
    // timer functions
	os->timer_usleep = win_udelay;
	os->timer_msleep = win_msleep;

    // interrupt functions
	os->os_intr_remove_affinity = win_os_intr_remove_affinity;
	os->os_intr_restore_affinity = win_os_intr_restore_affinity;
	os->intr_local_irq_enable = NULL;
	os->intr_local_irq_disable = NULL;
	os->intr_trigger_interrupt = win_intr_trigger_interrupt;
	os->sos_os_intr_probe_and_register = win_sos_os_intr_probe_and_register;
	os->sos_os_intr_unregister = win_sos_os_intr_unregister;
	os->sos_os_intr_get_vector = win_sos_os_intr_get_vector;
	os->intr_timer = win_intr_timer;
	os->sos_os_intr_get_os_lapic_id = win_sos_os_intr_get_os_lapic_id;

	// SecBase hypercalls
	os->secbase_test_presence = win_secbase_test_presence;
	os->hypercall_0arg = win_hypercall_0arg;
	os->hypercall_1arg = win_hypercall_1arg;
	os->hypercall_2args = win_hypercall_2args;
	os->hypercall_3args = win_hypercall_3args;
	os->hypercall_4args = win_hypercall_4args;
    os->hypercall_ret1 = win_hypercall_ret1;

	// Debug
	os->int3_debug_break = win_int3_debug_break;
	


	// Initialize Windows-specific configurations
	win_cfgs_init();

    // Init the memory manager for the S domain.
    sa_s_mem_init();
}

void os_fini(void)
{
    // Finalize the memory manager for the S domain
    sa_s_mem_fini();

	// Finalize Windows-specific configurations
	win_cfgs_fini();
}