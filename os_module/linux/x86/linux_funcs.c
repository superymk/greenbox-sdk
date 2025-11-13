// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// Linux specific implementation
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/rcupdate.h> //rcu_read_lock
#include <asm/siginfo.h>    //siginfo

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h> // send_sig_info
                                // typedef struct kernel_siginfo LINUX_STRUCT_SIGINFO;
#else
#include <linux/sched.h> // send_sig_info
// typedef struct siginfo LINUX_STRUCT_SIGINFO;
#endif

#include "linux_cpu.h"
#include "linux_mm.h"
#include "linux_intr.h"
#include "linux_event_notify.h"

bool cpu_support_clflush = false;

static void linux_cfgs_init(void)
{
    uint32_t eax, edx;

    // Support CLFLUSH?
    cpuid_eaxedx(1, &eax, &edx);

    if (edx & CPUID_FLAG_CLFL)
        cpu_support_clflush = true;
}

static void linux_cfgs_fini(void)
{
    cpu_support_clflush = false;
}

static void linux_sos_boot_params_init(struct sos_boot_params *boot_params)
{
    boot_params->sos_compat_conf = SOS_COMPATIBILITY_LINUX;
}

static process_id linux_get_current_pid(void)
{
    return (process_id)current->tgid;
}

static uint32_t linux_read_processor_id(void)
{
    return (uint32_t)smp_processor_id();
}

/********* Public Functions *********/
void os_init(void)
{
    os->sos_boot_params_init = linux_sos_boot_params_init;

    // cpu functions
    os->processor_id = linux_read_processor_id;
    os->read_msr = linux_read_msr;
    os->write_msr = linux_write_msr;
    os->atomic_increase = linux_atomic_increase;
    os->atomic_decrease = linux_atomic_decrease;
    os->spinlock_init = linux_spin_lock_init;
    os->spinlock_lock = linux_spin_lock;
    os->spinlock_trylock = linux_spin_trylock;
    os->spinlock_unlock = linux_spin_unlock;
    os->udelay = linux_udelay;
    os->inb = linux_inb;
    os->inw = linux_inw;
    os->inl = linux_inl;
    os->outb = linux_outb;
    os->outw = linux_outw;
    os->outl = linux_outl;

    // process functions
    os->get_current_pid = linux_get_current_pid;

    // memory functions
    os->mm_malloc_zeroed = linux_mm_malloc_zeroed;
    os->mm_free = linux_mm_free;
    os->mm_fence = linux_mm_fence;
    os->mm_vaddr_to_paddr = linux_mm_vaddr_to_paddr;
    os->mm_alloc_pages = linux_mm_alloc_pages;
    os->mm_alloc_physical_contiguous_pages = linux_mm_alloc_physical_contiguous_pages;
    os->mm_alloc_physical_contiguous_pages_nocache = linux_mm_alloc_physical_contiguous_pages_nocache;
    os->mm_free_pages = linux_mm_free_pages;
    os->mm_free_physical_contiguous_pages = linux_mm_free_physical_contiguous_pages;
    os->mm_ioremap_nocache = linux_ioremap_nocache;
    os->mm_iounmap = linux_iounmap;
    os->mm_alloc_and_lock_pages_for_sdom = NULL;
    os->mm_free_and_unlock_pages_for_sdom_by_kvaddr = NULL;
    os->mm_free_and_unlock_pages_for_sdom_by_uvaddr = NULL;
    os->mm_memset = linux_memset;
    os->mm_memcpy = linux_memcpy;
    os->mm_check_current_process_page_permission = linux_mm_check_current_process_page_permission;
    os->copy_to_user = linux_copy_to_user;
    os->copy_from_user = linux_copy_from_user;
    os->wbinvd = wbinvd;
    os->cache_flush_range = linux_cache_flush_range;

    /********* For Asynchronous Notifications from SecAgent to Untrusted Applications (e.g., SecProcess Loaders) *********/
    os->os_app_notif_subsystem_init = linux_os_app_notif_subsystem_init;
    os->os_app_notif_subsystem_fini = linux_os_app_notif_subsystem_fini;
    os->os_app_notif_register = linux_os_app_notif_register;
    os->os_app_notif_unregister = linux_os_app_notif_unregister;
    os->os_notify_app = linux_signal_app;

    // timer functions
    os->timer_usleep = linux_udelay;
    os->timer_msleep = linux_msleep;

    // interrupt functions
#ifdef GREENBOX_USE_UXMHF64_V1
    os->os_intr_remove_affinity = linux_os_intr_remove_affinity;
    os->os_intr_restore_affinity = linux_os_intr_restore_affinity;
#endif // GREENBOX_USE_UXMHF64_V1
    os->intr_local_irq_enable = linux_intr_local_irq_enable;
    os->intr_local_irq_disable = linux_intr_local_irq_disable;
    os->intr_trigger_interrupt = linux_intr_trigger_interrupt;
    os->sos_os_intr_probe_and_register = linux_sos_os_intr_probe_and_register;
    os->sos_os_intr_unregister = linux_sos_os_intr_unregister;
    os->sos_os_intr_start = linux_sos_os_intr_start;
	os->sos_os_intr_stop = linux_sos_os_intr_stop;
    os->sos_os_intr_get_vector = linux_sos_os_intr_get_vector;
    os->intr_timer = linux_intr_timer;
    os->sos_os_intr_get_os_lapic_id = linux_sos_os_intr_get_os_lapic_id;

    // SecBase hypercalls
    os->secbase_test_presence = linux_secbase_test_presence;
    os->hypercall_0arg = linux_hypercall_0arg;
    os->hypercall_1arg = linux_hypercall_1arg;
    os->hypercall_2args = linux_hypercall_2args;
    os->hypercall_3args = linux_hypercall_3args;
    os->hypercall_4args = linux_hypercall_4args;
    os->hypercall_ret1 = linux_hypercall_ret1;
    os->hypercall_ret2 = linux_hypercall_ret2;
    os->hypercall_ret3 = linux_hypercall_ret3;
    os->hypercall_ret4 = linux_hypercall_ret4;

    // Initialize Linux-specific configurations
    linux_cfgs_init();
}

void os_fini(void)
{
    // Finalize Linux-specific configurations
    linux_cfgs_fini();
}