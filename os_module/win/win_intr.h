// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_INTR_H
#define SECAGENT_WIN_INTR_H

#include "win_cpu.h"
#include <sa_headers.h>

/// @brief Trigger an INT3 breakpoint for debugging
extern void win_int3_debug_break(void);

/// @brief Trigger a software interrupt with a given vector
/// @param vector Interrupt vector
extern void win_intr_trigger_interrupt(uint32_t vector);

/// @brief Enable local CPU interrupts
extern void win_intr_local_irq_enable(void);

/// @brief Disable local CPU interrupts
extern void win_intr_local_irq_disable(void);

/// @brief Dispatch routine for interrupt create IRP
/// @param DeviceObject Device object
/// @param Irp IRP request
/// @return NTSTATUS
extern NTSTATUS InterruptCreateDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

/// @brief Initialize Windows interrupt subsystem for SecAgent
/// @param in_DriverObject Driver object
/// @param in_DeviceObject Device object
/// @param in_WDFDevice WDF device pointer
extern void win_intr_init(PDRIVER_OBJECT in_DriverObject, PDEVICE_OBJECT in_DeviceObject, WDFDEVICE* in_WDFDevice);

/// @brief Finalize Windows interrupt subsystem
extern void win_intr_fini(void);

/// @brief Probe and register an interrupt handler for SecOS-OS notifications
/// @return Return INVALID_INTR on errors
extern OSSTATUS win_sos_os_intr_probe_and_register(void);

/// @brief Unregister the interrupt handler for SecOS-OS notifications
extern void win_sos_os_intr_unregister(void);

/// @brief Start SecOS-OS interrupt handling
extern void win_sos_os_intr_start(void);

/// @brief Stop SecOS-OS interrupt handling
extern void win_sos_os_intr_stop(void);

/// @brief Return the interrupt vector for SecOS-OS notifications
/// @return Interrupt vector
extern intr_t win_sos_os_intr_get_vector(void);

/// @brief Timer interrupt handler entry
/// @return Interrupt vector
extern intr_t win_intr_timer(void);

/// @brief Get OS LAPIC ID for SecOS-OS interrupt
/// @return LAPIC ID
extern lapic_id_t win_sos_os_intr_get_os_lapic_id(void);

#ifdef GREENBOX_USE_UXMHF64_V1
/// @brief Remove OS interrupt affinity (UxMHF64 v1)
extern void win_os_intr_remove_affinity(void);

/// @brief Restore OS interrupt affinity (UxMHF64 v1)
extern void win_os_intr_restore_affinity(void);
#endif // GREENBOX_USE_UXMHF64_V1


/// @brief Get current IRQ priority for debugging
extern void win_debug_intr_get_current_irq_priority(void);

#endif // SECAGENT_WIN_INTR_H

