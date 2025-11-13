// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_LINUX_INTR_H
#define SECAGENT_LINUX_INTR_H

#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern void linux_intr_trigger_interrupt(uint32_t vector);

extern void linux_intr_local_irq_enable(void);
extern void linux_intr_local_irq_disable(void);

/// @brief Probe and register an interrupt handler for SecOS-OS notifications. 
/// @param  
/// @return
extern OSSTATUS linux_sos_os_intr_probe_and_register(void);

/// @brief Unregister the interrupt handler for SecOS-OS notifications.
/// @param  
extern void linux_sos_os_intr_unregister(void);

extern void linux_sos_os_intr_start(void);
extern void linux_sos_os_intr_stop(void);

/// @brief Return the interrupt vector for SecOS-OS notifications.
/// @param  
/// @return 
extern intr_t linux_sos_os_intr_get_vector(void);
extern intr_t linux_intr_timer(void);
extern lapic_id_t linux_sos_os_intr_get_os_lapic_id(void);

extern void linux_os_intr_read_affinity(void);

#ifdef GREENBOX_USE_UXMHF64_V1
extern void linux_os_intr_remove_affinity(void);
extern void linux_os_intr_restore_affinity(void);
#endif // GREENBOX_USE_UXMHF64_V1


/********* Signal Functions *********/
extern void linux_signal_app(process_id pid);

#endif // __ASSEMBLY__
#endif // SECAGENT_LINUX_INTR_H