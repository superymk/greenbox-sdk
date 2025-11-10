// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_INTR_H
#define SECAGENT_WIN_INTR_H

#include "win_cpu.h"
#include <sa_headers.h>

extern void win_int3_debug_break(void);
extern void win_intr_trigger_interrupt(uint32_t vector);

extern void win_intr_local_irq_enable(void);
extern void win_intr_local_irq_disable(void);

extern NTSTATUS InterruptCreateDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

extern void win_intr_init(PDRIVER_OBJECT in_DriverObject, PDEVICE_OBJECT in_DeviceObject, WDFDEVICE* in_WDFDevice);
extern void win_intr_fini(void);

/// @brief Probe and register an interrupt handler for SecOS-OS notifications. 
/// @param  
/// @return Return INVALID_INTR on errors
extern OSSTATUS win_sos_os_intr_probe_and_register(void);

/// @brief Unregister the interrupt handler for SecOS-OS notifications.
/// @param  
extern void win_sos_os_intr_unregister(void);

extern void win_sos_os_intr_start(void);
extern void win_sos_os_intr_stop(void);

/// @brief Return the interrupt vector for SecOS-OS notifications.
/// @param  
/// @return 
extern intr_t win_sos_os_intr_get_vector(void);

extern intr_t win_intr_timer(void);

extern lapic_id_t win_sos_os_intr_get_os_lapic_id(void);

#ifdef GREENBOX_USE_UXMHF64_V1
extern void win_os_intr_remove_affinity(void);
extern void win_os_intr_restore_affinity(void);
#endif // GREENBOX_USE_UXMHF64_V1


extern void win_debug_intr_get_current_irq_priority(void);

#endif // SECAGENT_WIN_INTR_H

