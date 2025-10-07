// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

#define IOAPIC_NUM_ENTRIES   (24)

extern void os_send_ipi_to_cpu(u8 vector, lapic_id_t apic_id);

// static int sos_os_irq = INVALID_IRQ;
static lapic_id_t secos_lapic_id = INVALID_LAPIC_ID;
struct IO_APIC_entry os_ioapic_entries[IOAPIC_NUM_ENTRIES] = {{0}};

static uint32_t os_x86_ioapic_read(struct io_apic *ioapic, uint32_t reg)
{
    sa_write32((vaddr_t)&ioapic->index, reg);
    return sa_read32((vaddr_t)&ioapic->data);
}

static void os_x86_ioapic_write(struct io_apic *ioapic, uint32_t reg, uint32_t value)
{
	sa_write32((vaddr_t)&ioapic->index, reg);
	sa_write32((vaddr_t)&ioapic->data, value);
}


static struct IO_APIC_entry os_ioapic_read_entry(struct io_apic *ioapic, int pin)
{
    union entry_union eu;

    eu.w1 = os_x86_ioapic_read(ioapic, 0x10 + 2 * pin);
    eu.w2 = os_x86_ioapic_read(ioapic, 0x11 + 2 * pin);

    return eu.entry;
}

static void os_ioapic_write_entry(struct io_apic *ioapic, int pin, struct IO_APIC_entry *entry)
{
	union entry_union *eu = (union entry_union *)entry;

	os_x86_ioapic_write(ioapic, 0x10 + 2 * pin, eu->w1);
	os_x86_ioapic_write(ioapic, 0x11 + 2 * pin, eu->w2);
}

// struct IO_APIC_entry os_ioapic_entrys[IOAPIC_NUM_ENTRIES];
// void ioapic_read(paddr_t ioapic_paddr)
// {
//     int i = 0;
//     struct io_apic *ioapic;

//     ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);
//     DRV_DEBUG("ioapic vaddr:0x%lX\n", (vaddr_t)ioapic);

//     for (i = 0; i < IOAPIC_NUM_ENTRIES; i++)
//     {
//         struct IO_APIC_entry entry;

//         entry = os_ioapic_read_entry(ioapic, i);
//         os_ioapic_entrys[i] = entry;
//     }

//     os->mm_iounmap(ioapic, PAGE_SIZE);
// }

//! Get the physical address of the IOAPIC registers page
//! make sure you map it to virtual memory
paddr_t ioapic_get_base(void)
{
    return 0xfec00000;
}

intr_t ioapic_get_vector(int irq)
{
    struct io_apic *ioapic;
    paddr_t ioapic_paddr = ioapic_get_base();
    struct IO_APIC_entry entry;

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);

    entry = os_ioapic_read_entry(ioapic, irq);

    os->mm_iounmap((void *)ioapic, PAGE_SIZE);

    return (intr_t)entry.vector;
}

uint32_t ioapic_get_irq_affinity(int irq, bool* is_dest_physical_mode)
{
    struct io_apic *ioapic;
    paddr_t ioapic_paddr = ioapic_get_base();
    struct IO_APIC_entry entry;

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);

    entry = os_ioapic_read_entry(ioapic, irq);

    os->mm_iounmap((void *)ioapic, PAGE_SIZE);

    // Return results
    if(is_dest_physical_mode)
        *is_dest_physical_mode = (entry.dest_mode == 0) ? true : false;

    return entry.dest;
}

// void ioapic_set_irq_affinity(int irq)
// {
//     struct io_apic *ioapic;
//     paddr_t ioapic_paddr = ioapic_get_base();
//     struct IO_APIC_entry entry;

//     ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);
//     entry = os_ioapic_read_entry(ioapic, irq);

//     entry.dest = 0x0F;
//     entry.trigger = 1;

//     os_ioapic_write_entry(ioapic, irq, &entry);
//     os->mm_iounmap((void *)ioapic, PAGE_SIZE);
// }

void ioapic_dump_entries(void)
{
    int i = 0;
    struct io_apic *ioapic;
    paddr_t ioapic_paddr = ioapic_get_base();

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);

    DRV_INFO(" NR Dst Mask Trig RIRR Pol Stat Dmod Deli Vect:\n");
    for (i = 0; i < IOAPIC_NUM_ENTRIES; i++)
    {
        struct IO_APIC_entry entry;

        entry = os_ioapic_read_entry(ioapic, i);

        DRV_INFO(" %02x %02X  %1d	 %1d	%1d   %1d	%1d    "
                 "%1d	 %1d	%02X\n",
                 i, entry.dest,
                 entry.mask,
                 entry.trigger,
                 entry.rirr,
                 entry.polarity,
                 entry.delivery_status,
                 entry.dest_mode,
                 entry.delivery_mode,
                 entry.vector);
    }

    os->mm_iounmap((void *)ioapic, PAGE_SIZE);
}

#define IA32_X2APIC_LDR 0x00D
#define IA32_APIC_BASE_MSR_EXTD (1UL << 10)
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define IA32_X2APIC_ICR 0x830
#define	APIC_TASKPRI	0x80
#define	APIC_PROCPRI	0xA0

static bool _lapic_is_x2apic(void)
{
    uint64_t msr = 0;

    msr = os->read_msr(IA32_APIC_BASE_MSR);
    if ((msr & IA32_APIC_BASE_MSR_EXTD) && (msr & IA32_APIC_BASE_MSR_ENABLE))
        return true;

    return false;
}

static bool _lapic_is_xapic(void)
{
    uint64_t msr = 0;

    msr = os->read_msr(IA32_APIC_BASE_MSR);
    if ((!(msr & IA32_APIC_BASE_MSR_EXTD)) && (msr & IA32_APIC_BASE_MSR_ENABLE))
        return true;

    return false;
}

void os_send_ipi_to_cpu(u8 vector, lapic_id_t apic_id)
{
    volatile uint32_t timeout = 0x00100000;
    paddr_t lapic_base_paddr;
    vaddr_t lapic_base_vaddr;

    if (_lapic_is_x2apic())
    {
        // Found x2APIC
        uint32_t low = (0x00004000UL | vector);
        uint32_t high = apic_id;

        os->write_msr(IA32_X2APIC_ICR, low, high);
    }
    else if (_lapic_is_xapic())
    {
        os->mm_fence();

        lapic_base_paddr = lapic_get_base();
        lapic_base_vaddr = (vaddr_t)os->mm_ioremap_nocache(lapic_base_paddr, PAGE_SIZE);

#define LAPIC_ID_SHIFT (24)
        sa_write32(lapic_base_vaddr + 0x310, (uint32_t)(apic_id << LAPIC_ID_SHIFT));
        sa_write32(lapic_base_vaddr + 0x300, (uint32_t)(0x00005000 | vector));

        os->mm_fence();

        {
            volatile uint32_t val;
            do
            {
                val = sa_read32(lapic_base_vaddr + 0x300);
                os->udelay(1);
            } while (--timeout > 0 && (val & 0x1000));
        }

        os->mm_fence();
        os->mm_iounmap((void *)lapic_base_vaddr, PAGE_SIZE);
    }

    SACOMM_DEBUG("Sent IPI %u to SecOS on LAPIC %u\n", vector, apic_id);
}

static void _os_intr_read_ioapic_affinity_in_ioapic(void)
{
    OSSTATUS status = OS_SUCCESS;

	status = x86_ioapic_read_entire_table(os_ioapic_entries);
	if(SA_FAIL(status))
	{
		DRV_ERR("Failed to read affinity in IOAPIC\n");
	}
}

static void _os_intr_restore_affinity_in_ioapic(void)
{
    OSSTATUS status = OS_SUCCESS;

	status = x86_ioapic_write_entire_table(os_ioapic_entries);
	if(SA_FAIL(status))
	{
		DRV_ERR("Failed to restore affinity in IOAPIC of CPU 1\n");
	}
}




/********* Public Functions *********/
OSSTATUS sa_init_secos_os_comm_intr(void)
{
    ulong_t ret = 0;
    ulong_t val = 0;

    // Read OS's configuration of IOAPIC
    _os_intr_read_ioapic_affinity_in_ioapic();

    // // Remove interrupt affinity to this core
    // os->os_intr_remove_affinity();

    ret = os->hypercall_ret1(NS_HC_GET_LAPIC_ID, &val);
    if (ret)
    {
        DRV_ERR("Failed to get LAPIC ID of the current CPU!\n");
        return E_INTERNAL_ERROR;
    }

    secos_lapic_id = (lapic_id_t)val;
    return OS_SUCCESS;
}

void sa_fini_secos_os_comm_intr(void)
{
    secos_lapic_id = INVALID_LAPIC_ID;

    // // Restore interrupt affinity to this core
    // os->os_intr_restore_affinity();
    // _os_intr_restore_affinity_in_ioapic();
}

// OSSTATUS sa_register_sos_os_irq(void)
// {
//     OSSTATUS status = OS_SUCCESS;

//     if(sos_os_irq != INVALID_IRQ)
//         return EINVAL;

//     sos_os_irq = os->find_sos_os_irq();
//     if(sos_os_irq == INVALID_IRQ)
// 	{
// 		DRV_ERR("Register SecOS-OS IRQ Error: No free IRQ!\n"); 
// 		goto find_irq_err;
// 	}

//     status = os->intr_register(sos_os_irq);
// 	if(SA_FAIL(status))
// 	{
// 		DRV_ERR("Register SecOS-OS IRQ Error: Failed to claim IRQ:%d\n", sos_os_irq); 
// 		goto irq_register_err;
// 	}

//     // On success
//     return OS_SUCCESS;


// irq_register_err:
// find_irq_err:
//     sos_os_irq = INVALID_IRQ;
//     return E_INTERNAL_ERROR;
// }

// void sa_unregister_sos_os_irq(void)
// {
//     if(sos_os_irq != INVALID_IRQ)
// 	{
// 		os->intr_unregister(sos_os_irq);
// 		sos_os_irq = INVALID_IRQ;
// 	}
// }

// int sa_get_sos_os_irq(void)
// {
//     return sos_os_irq;
// }

void notify_secos(void)
{
    OSSTATUS status = OS_SUCCESS;
    intr_t vector = os->sos_os_intr_get_vector();
    VKSTATUS sos_status = SUCCESS;

    if(vector == INVALID_INTR)
    {
        DRV_ERR("Unknown interrupt vector of SecOS to notify!\n"); 
        return;
    }

    if(secos_lapic_id == INVALID_LAPIC_ID)
    {
        DRV_ERR("SecOS-OS communication channel has been destroyed!\n"); 
        return;
    }

    // Send IPI to SecOS if its status is <SECOS_INFO_RETURN_FROM_PREEMPTION>
    status = sos_get_status(&sos_status);
    if(SA_PASS(status) && (sos_status == SECOS_INFO_RETURN_FROM_PREEMPTION))
        os_send_ipi_to_cpu(vector, secos_lapic_id);
}

OSSTATUS x86_ioapic_read_entire_table(struct IO_APIC_entry *out_entries)
{
    struct io_apic *ioapic;
	int i = 0;
    paddr_t ioapic_paddr = ioapic_get_base();

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);
	DRV_DEBUG("<x86_ioapic_read_entire_table> Read IOAPIC table\n");
	for (i = 0; i < IOAPIC_NUM_ENTRIES; i++)
	{
		struct IO_APIC_entry entry;

		entry = os_ioapic_read_entry(ioapic, i);
		out_entries[i] = entry;
	}
    os->mm_iounmap((void *)ioapic, PAGE_SIZE);

    DRV_INFO("[Superymk] <x86_ioapic_read_entire_table>\n");
	ioapic_dump_entries();
	return OS_SUCCESS;
}

OSSTATUS x86_ioapic_write_entire_table(struct IO_APIC_entry *entries)
{
    struct io_apic *ioapic;
	int i = 0;
	paddr_t ioapic_paddr = ioapic_get_base();

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);
	for (i = 0; i < IOAPIC_NUM_ENTRIES; i++)
	{
		os_ioapic_write_entry(ioapic, i, &entries[i]);
	}

    DRV_INFO("[Superymk] <x86_ioapic_write_entire_table>\n");
	DRV_DEBUG("<x86_ioapic_restore_OS_configs> Restoring IOAPIC\n");
	ioapic_dump_entries();
	return OS_SUCCESS;
}

// Disable delivering IOAPIC interrupt delivery to the indicated core
OSSTATUS ioapic_remove_allintrs_affinity(uint32_t logical_apic_id)
{
    struct io_apic *ioapic;
    paddr_t ioapic_paddr = ioapic_get_base();
	u8 mask = 0;
	u8 i = 0;
	struct IO_APIC_entry entry;

	if (logical_apic_id >= 8)
	{
		DRV_ERR("Remove affinity in IOAPIC error! SecOS's IOAPIC code only supports 8 CPUs at most\n");
		return E_INV_ARG;
	}
	mask = ~(1 << logical_apic_id);

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);
	DRV_DEBUG("<ioapic_remove_allintrs_affinity> Disable IRQ delivery to logical APIC ID:%u\n", logical_apic_id);
	for (i = 0; i < IOAPIC_NUM_ENTRIES; i++)
	{
		entry = os_ioapic_read_entry(ioapic, i);

		if (entry.vector != 0xFF)
		{
			entry.dest &= mask;

            // Update the interrupt affinity if the new value still binds the interrupt to at least one CPU
            if(entry.dest)
			    os_ioapic_write_entry(ioapic, i, &entry);
		}
	}
    os->mm_iounmap((void *)ioapic, PAGE_SIZE);

    DRV_INFO("[Superymk] <ioapic_remove_allintrs_affinity>\n");
	ioapic_dump_entries();
	return OS_SUCCESS;
}

// Set delivering IOAPIC interrupt delivery to the indicated core
OSSTATUS ioapic_add_allintrs_affinity(uint32_t logical_apic_id)
{
    struct io_apic *ioapic;
    paddr_t ioapic_paddr = ioapic_get_base();
	u8 mask = 0;
	u8 i = 0;
	struct IO_APIC_entry entry;

	if (logical_apic_id >= 8)
	{
		DRV_ERR("Add affinity in IOAPIC error! SecOS's IOAPIC code only supports 8 CPUs at most\n");
		return E_INV_ARG;
	}
	mask = (1 << logical_apic_id);

    ioapic = (struct io_apic *)os->mm_ioremap_nocache(ioapic_paddr, PAGE_SIZE);
	DRV_DEBUG("<ioapic_add_allintrs_affinity> Enable IRQ delivery to logical APIC ID:%u\n", logical_apic_id);
	for (i = 0; i < IOAPIC_NUM_ENTRIES; i++)
	{
		entry = os_ioapic_read_entry(ioapic, i);

		if (entry.vector != 0xFF)
		{
			entry.dest |= mask;
			os_ioapic_write_entry(ioapic, i, &entry);
		}
	}
    os->mm_iounmap((void *)ioapic, PAGE_SIZE);

    DRV_INFO("[Superymk] <ioapic_add_allintrs_affinity>\n");
	ioapic_dump_entries();
	return OS_SUCCESS;
}

uint32_t lapic_read_tpr(void)
{
    paddr_t lapic_base_paddr = INVALID_PADDR;
    vaddr_t lapic_base_vaddr = INVALID_VADDR;

    if (_lapic_is_x2apic())
    {
        // Found x2APIC
        return (uint32_t)os->read_msr(APIC_TASKPRI);
    }
    else if (_lapic_is_xapic())
    {
        volatile uint32_t result = 0;
        os->mm_fence();

        lapic_base_paddr = lapic_get_base();
        lapic_base_vaddr = (vaddr_t)os->mm_ioremap_nocache(lapic_base_paddr, PAGE_SIZE);

        //result = *(volatile uint32_t *)((lapic_base_vaddr & PAGE_MASK_4K) + APIC_TASKPRI);
        result = sa_read32(lapic_base_vaddr + APIC_TASKPRI);

        os->mm_fence();

        os->mm_iounmap((void *)lapic_base_vaddr, PAGE_SIZE);

        return result;
    }

    DRV_ERR("<lapic_read_tpr> Unknown LAPIC!\n");
    return 0xFFFFFFFF;
}

uint32_t lapic_read_ppr(void)
{
    paddr_t lapic_base_paddr = INVALID_PADDR;
    vaddr_t lapic_base_vaddr = INVALID_VADDR;

    if (_lapic_is_x2apic())
    {
        // Found x2APIC
        return (uint32_t)os->read_msr(APIC_PROCPRI);
    }
    else if (_lapic_is_xapic())
    {
        volatile uint32_t result = 0;
        os->mm_fence();

        lapic_base_paddr = lapic_get_base();
        lapic_base_vaddr = (vaddr_t)os->mm_ioremap_nocache(lapic_base_paddr, PAGE_SIZE);

        // result = *(volatile uint32_t *)((lapic_base_vaddr & PAGE_MASK_4K) + APIC_PROCPRI);
        result = sa_read32(lapic_base_vaddr + APIC_PROCPRI);

        os->mm_fence();

        os->mm_iounmap((void *)lapic_base_vaddr, PAGE_SIZE);

        return result;
    }

    DRV_ERR("<lapic_read_ppr> Unknown LAPIC!\n");
    return 0xFFFFFFFF;
}

OSSTATUS do_CMD_SECAGENT_X86_OS_IRQ_GET_INTR_VECTOR(struct tz_cmd_packet *cmd_packet)
{
    uint32_t irq_no = 0;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;

    // Check: Input IRQ number must be less than IOAPIC_NUM_ENTRIES
    irq_no = (uint32_t)cmd_packet->arg1;
    if(irq_no >= IOAPIC_NUM_ENTRIES)
        return E_INV_ARG;

    cmd_packet->arg1 = os_ioapic_entries[irq_no].vector;

    return OS_SUCCESS;
}