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

extern void win_intr_init(PDRIVER_OBJECT in_DriverObject, PDEVICE_OBJECT in_DeviceObject);
extern void win_intr_fini(void);

/// @brief Probe and register an interrupt handler for SecOS-OS notifications. 
/// @param  
/// @return Return INVALID_INTR on errors
extern OSSTATUS win_sos_os_intr_probe_and_register(void);

/// @brief Unregister the interrupt handler for SecOS-OS notifications.
/// @param  
extern void win_sos_os_intr_unregister(void);

/// @brief Return the interrupt vector for SecOS-OS notifications.
/// @param  
/// @return 
extern intr_t win_sos_os_intr_get_vector(void);

extern intr_t win_intr_timer(void);

extern lapic_id_t win_sos_os_intr_get_os_lapic_id(void);

extern void win_os_intr_remove_affinity(void);
extern void win_os_intr_restore_affinity(void);



extern void win_debug_intr_get_current_irq_priority(void);

#endif // SECAGENT_WIN_INTR_H

