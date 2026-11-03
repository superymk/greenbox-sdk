// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_APIC_H
#define SECAGENT_APIC_H
#include <os_types.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#ifndef __ASSEMBLY__

struct io_apic
{
    unsigned int index;
    unsigned int unused[3];
    unsigned int data;
    unsigned int unused2[11];
    unsigned int eoi;
};

struct IO_APIC_entry
{
    uint32_t vector : 8,
        delivery_mode : 3, /* 000: FIXED
                            * 001: lowest prio
                            * 111: ExtINT
                            */
        dest_mode : 1,     /* 0: physical, 1: logical */
        delivery_status : 1,
        polarity : 1,
        rirr : 1,    // remote irr
        trigger : 1, /* 0: edge, 1: level */
        mask : 1,    /* 0: enabled, 1: disabled */
        __reserved_2 : 15;

    uint32_t __reserved_3 : 24,
        dest : 8;
};

union entry_union
{
    struct
    {
        uint32_t w1, w2;
    };
    struct IO_APIC_entry entry;
};

/// @brief Init SecOS-OS communication channel's interrupt configurations
/// @return OS_SUCCESS on success. E_INTERNAL_ERROR if not.
extern OSSTATUS sa_init_secos_os_comm_intr(void);

/// @brief Finalize SecOS-OS communication channel's interrupt configurations
extern void sa_fini_secos_os_comm_intr(void);

/// @brief Send IPI to SecOS
extern void notify_secos(void);

/// @brief Get the physical address of the APIC registers page
/// @return Physical address
extern paddr_t lapic_get_base(void);

/// @brief Get the physical address of the IOAPIC registers page
/// @return Physical address
extern paddr_t ioapic_get_base(void);

// extern void ioapic_set_irq_affinity(int irq);
/// @brief Get interrupt vector for an IRQ
/// @param irq IRQ number
/// @return Interrupt vector
extern intr_t ioapic_get_vector(int irq);

/// @brief Get IRQ affinity
/// @param irq IRQ number
/// @param is_dest_physical_mode Output flag for physical mode
/// @return Destination APIC ID
extern uint32_t ioapic_get_irq_affinity(int irq, bool *is_dest_physical_mode);

/// @brief Dump IOAPIC entries
extern void ioapic_dump_entries(void);

// extern void ioapic_read(paddr_t ioapic_paddr);

/// @brief Read entire IOAPIC table
/// @param out_entries Output entries
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS x86_ioapic_read_entire_table(struct IO_APIC_entry *out_entries);

/// @brief Write entire IOAPIC table
/// @param entries Entries to write
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS x86_ioapic_write_entire_table(struct IO_APIC_entry *entries);
/// @brief Disable delivering IOAPIC interrupt delivery to the indicated core
/// @param logical_apic_id Logical APIC ID
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS ioapic_remove_allintrs_affinity(uint32_t logical_apic_id);

/// @brief Enable delivering IOAPIC interrupt delivery to the indicated core
/// @param logical_apic_id Logical APIC ID
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS ioapic_add_allintrs_affinity(uint32_t logical_apic_id);

/// @brief Read the Task-Priority Register of the current CPU's LAPIC
/// @return TPR value
extern uint32_t lapic_read_tpr(void);

/// @brief Read the Processor-Priority Register of the current CPU's LAPIC
/// @return PPR value
extern uint32_t lapic_read_ppr(void);

/// @brief Get OS IRQ interrupt vector
/// @param cmd_packet Command packet
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS do_CMD_SECAGENT_X86_OS_IRQ_GET_INTR_VECTOR(struct tz_cmd_packet *cmd_packet);

#endif // __ASSEMBLY__
#endif // SECAGENT_APIC_H