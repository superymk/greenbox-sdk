// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// @file
#ifndef OS_FUNCS_H
#define OS_FUNCS_H

#include <sos_common_defs.h>

// mm_alloc_physical_contiguous_pages
#define ALLOC_ORDINARY_MEMORY		1
#define ALLOC_DMA_MEMORY			2

#ifndef __ASSEMBLY__

/// Forward declarations
struct sa_app_notif_reginfo;
struct sos_boot_params;

//! \brief This struct abstract the required common OS functions
//!
//! When porting the SecAgent to a new OS, these functions must be implemented
//! for that specific OS.
struct os_funcs
{
	void (*sos_boot_params_init)(struct sos_boot_params* boot_params);
	
	// cpu functions
	uint32_t (*processor_id)(void);
	uint64_t (*read_msr)(uint32_t msr);
	void (*write_msr)(uint32_t reg, uint32_t low, uint32_t high);
	void (*atomic_increase)(uint32_t* var);
    void (*atomic_decrease)(uint32_t* var);

    void (*spinlock_init)(volatile lock_t* lock);
	/// @brief Acquire the spin lock for OS kernel. The function may disable interrupts. The caller must release the
    /// lock before returning to the user mode.
	void (*spinlock_lock)(volatile lock_t* lock);
    /// @brief Try to acquire the spin lock for OS kernel. The function may disable interrupts. The caller must release 
    /// the lock before returning to the user mode.
    bool (*spinlock_trylock)(volatile lock_t* lock);
	void (*spinlock_unlock)(volatile lock_t* lock);
	void (*udelay)(uint32_t usec);
	uint8_t (*inb)(uint16_t port);
	uint16_t (*inw)(uint16_t port);
	uint32_t (*inl)(uint16_t port);
	void (*outb)(uint8_t value, uint16_t port);
	void (*outw)(uint16_t value, uint16_t port);
	void (*outl)(uint32_t value, uint16_t port);

    // Process functions
	/// @brief Return the PID of the *root* process of the current SecProcess loader.
	/// [NOTE] The returned PID is not the thread ID of the calling thread. A SecProcess loader has multiple threads in 
	/// most cases. OS kernels must report the parent process ID of these threads, not the thread ID of the thread
	/// calling this function.
    process_id (*get_current_pid)(void);

	// memory functions
	//! Return NULL on error
	void* (*mm_malloc_zeroed)(size_t size);
	void (*mm_free)(void* ptr);
	void (*mm_fence)(void);
	paddr_t (*mm_vaddr_to_paddr)(vaddr_t vaddr);
	vaddr_t (*mm_alloc_pages)(uint32_t num_pages, uint32_t type);
	vaddr_t (*mm_alloc_physical_contiguous_pages)(uint32_t num_pages, uint32_t type);
	vaddr_t (*mm_alloc_physical_contiguous_pages_nocache)(uint32_t num_pages, uint32_t type);
	void (*mm_free_pages)(vaddr_t vaddr, uint32_t num_pages, uint32_t type);
	void (*mm_free_physical_contiguous_pages)(vaddr_t vaddr, uint32_t num_pages, uint32_t type);
	void* (*mm_ioremap_nocache)(paddr_t paddr, size_t size);
	void (*mm_iounmap)(void* ptr, size_t size);

    void* (*mm_alloc_and_lock_pages_for_sdom)(uint32_t num_pages, uvaddr_t* out_uvaddr);
    void (*mm_free_and_unlock_pages_for_sdom_by_kvaddr)(void* ptr);
    void (*mm_free_and_unlock_pages_for_sdom_by_uvaddr)(uvaddr_t uvaddr);

	void (*mm_memset)(void* ptr, char v, size_t size);
	void (*mm_memcpy)(void *__to, const void *__from, size_t __n);
	OSSTATUS (*mm_check_current_process_page_permission)(ulong_t uvaddr, size_t size, bool* out_allowed_read_write);
	//! Copy data from kernel space to user space.
	//! Returns number of bytes that could not be copied. On success, this will be zero.
	OSSTATUS (*copy_to_user)(void* usr_dest, const void* kern_src, size_t size);
	//! Copy data from user space to kernel space.
	//! Returns number of bytes that could not be copied. On success, this will be zero.
	//! If some data could not be copied, this function will pad the copied data to the requested size using zero bytes.
	OSSTATUS (*copy_from_user)(void* kern_dest, const void* usr_src, size_t size);
	void (*wbinvd)(void);
	OSSTATUS (*cache_flush_range)(vaddr_t vaddr, size_t size);

    /********* For Asynchronous Notifications from SecAgent to Untrusted Applications (e.g., SecProcess Loaders) *********/
	void (*os_app_notif_subsystem_init)(void);
	void (*os_app_notif_subsystem_fini)(void);
	OSSTATUS (*os_app_notif_register)(struct sa_app_notif_reginfo* info);
	void (*os_app_notif_unregister)(void);
    void (*os_notify_app)(process_id pid);
	
	// timer functions
	void (*timer_usleep)(uint32_t us);
	void (*timer_msleep)(uint32_t ms);

	// interrupt
#ifdef GREENBOX_USE_UXMHF64_V1
	void (*os_intr_remove_affinity)(void);
	void (*os_intr_restore_affinity)(void);
#endif // GREENBOX_USE_UXMHF64_V1
	void (*intr_local_irq_enable)(void);
	void (*intr_local_irq_disable)(void);
	void (*intr_trigger_interrupt)(uint32_t vector);
	OSSTATUS (*sos_os_intr_probe_and_register)(void); // Probe and register an interrupt handler for SecOS-OS notifications. Return INVALID_INTR on errors
	void (*sos_os_intr_unregister)(void); // Unregister the interrupt handler for SecOS-OS notifications.
	void (*sos_os_intr_start)(void);
	void (*sos_os_intr_stop)(void);
	intr_t (*sos_os_intr_get_vector)(void);
	intr_t (*intr_timer)(void);
	lapic_id_t (*sos_os_intr_get_os_lapic_id)(void);

	// SecBase interfaces
	bool (*secbase_test_presence)(void);
	ulong_t (*hypercall_0arg)(ulong_t fn);
	ulong_t (*hypercall_1arg)(ulong_t fn, ulong_t arg1);
	ulong_t (*hypercall_2args)(ulong_t fn, ulong_t arg1, ulong_t arg2);
	ulong_t (*hypercall_3args)(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3);
	ulong_t (*hypercall_4args)(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4);
    ulong_t (*hypercall_ret1)(ulong_t fn, ulong_t* arg1);
	ulong_t (*hypercall_ret2)(ulong_t fn, ulong_t* arg1, ulong_t* arg2);
	ulong_t (*hypercall_ret3)(ulong_t fn, ulong_t* arg1, ulong_t* arg2, ulong_t* arg3);
	ulong_t (*hypercall_ret4)(ulong_t fn, ulong_t* arg1, ulong_t* arg2, ulong_t* arg3, ulong_t* arg4);

	// Debug
	void (*int3_debug_break)(void);
};

extern process_id os_get_current_pid(void);
extern void os_notify_app(process_id pid);

#endif //  __ASSEMBLY__
#endif // OS_FUNCS_H