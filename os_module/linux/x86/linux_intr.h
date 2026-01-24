// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_LINUX_INTR_H
#define SECAGENT_LINUX_INTR_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

/// @brief Trigger a software interrupt
/// @param vector Interrupt vector
extern void linux_intr_trigger_interrupt(uint32_t vector);

/// @brief Enable local IRQs
extern void linux_intr_local_irq_enable(void);

/// @brief Disable local IRQs
extern void linux_intr_local_irq_disable(void);

/// @brief Probe and register an interrupt handler for SecOS-OS notifications. 
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS linux_sos_os_intr_probe_and_register(void);

/// @brief Unregister the interrupt handler for SecOS-OS notifications.
extern void linux_sos_os_intr_unregister(void);

/// @brief Start SecOS-OS interrupt handling
extern void linux_sos_os_intr_start(void);
/// @brief Stop SecOS-OS interrupt handling
extern void linux_sos_os_intr_stop(void);

/// @brief Return the interrupt vector for SecOS-OS notifications.
/// @return Interrupt vector
extern intr_t linux_sos_os_intr_get_vector(void);

/// @brief Get timer interrupt vector
/// @return Timer interrupt vector
extern intr_t linux_intr_timer(void);

/// @brief Get OS LAPIC ID for SecOS-OS notifications
/// @return LAPIC ID
extern lapic_id_t linux_sos_os_intr_get_os_lapic_id(void);

/// @brief Read OS interrupt affinity
extern void linux_os_intr_read_affinity(void);

#ifdef GREENBOX_USE_UXMHF64_V1
/// @brief Remove OS interrupt affinity
extern void linux_os_intr_remove_affinity(void);

/// @brief Restore OS interrupt affinity
extern void linux_os_intr_restore_affinity(void);
#endif // GREENBOX_USE_UXMHF64_V1


/********* Signal Functions *********/
/// @brief Signal an application by PID
/// @param pid Process ID
extern void linux_signal_app(process_id pid);

#endif // __ASSEMBLY__
#endif // SECAGENT_LINUX_INTR_H