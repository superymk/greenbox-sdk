// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>


#define CPU_NUMS_MAX		(256)
#define INVALID_CPU_ID      (0xFFFFFFFF)

struct x86_os_cpu_lapic_id_mapping
{
    // <cpu_id> field is needed if red OS assigns non-contiguous CPU IDs.
	uint32_t cpu_id;
	lapic_id_t lapic_id;
};

static struct x86_os_cpu_lapic_id_mapping cpu_lapic_mappings[CPU_NUMS_MAX];

static void _x86_os_cpu_lapic_mapping_init_clear(void)
{
    uint32_t i = 0;

    FOREACH_S(i, CPU_NUMS_MAX, CPU_NUMS_MAX, 0, 1)
    {
        cpu_lapic_mappings[i].cpu_id = INVALID_CPU_ID;
        cpu_lapic_mappings[i].lapic_id = 0;
    }
}

static struct x86_os_cpu_lapic_id_mapping* _x86_os_cpu_lapic_mapping_find_slot(uint32_t cpu_id)
{
    uint32_t i = 0;

    FOREACH_S(i, CPU_NUMS_MAX, CPU_NUMS_MAX, 0, 1)
    {
        if(cpu_lapic_mappings[i].cpu_id == cpu_id)
            return &cpu_lapic_mappings[i]; 
    }

    return NULL;
}




/************ Public Functions **********/
void __x86_os_cpu_lapic_mapping_init(void)
{
    _x86_os_cpu_lapic_mapping_init_clear();
}

void __x86_os_cpu_lapic_mapping_fini(void)
{
    _x86_os_cpu_lapic_mapping_init_clear();
}

bool __x86_os_cpu_lapic_mapping_find(uint32_t cpu_id, lapic_id_t* out_lapic_id)
{
    struct x86_os_cpu_lapic_id_mapping* entry = NULL;

    if(!out_lapic_id)
        return false;

    entry = _x86_os_cpu_lapic_mapping_find_slot(cpu_id);
    if(!entry)
        return false;

    *out_lapic_id = entry->lapic_id;
    return true;
}

OSSTATUS do_CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU(struct tz_cmd_packet *cmd_packet)
{
	ulong_t ret = 0;
	ulong_t cur_lapic_id = 0;
    struct x86_os_cpu_lapic_id_mapping* entry = NULL;
    uint32_t cur_cpu_id = os->processor_id();

	UNUSED(cmd_packet);
	
	ret = os->hypercall_ret1(NS_HC_GET_LAPIC_ID, &cur_lapic_id);
    if (ret)
    {
        DRV_ERR("CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU error: Failed to get LAPIC ID of the current CPU!\n");
        return E_INTERNAL_ERROR;
    }

    // Register
    //// Check: Skip double registration
    entry = _x86_os_cpu_lapic_mapping_find_slot(cur_cpu_id);
    if(entry)
        return OS_SUCCESS;

    //// Check: Free space in <cpu_lapic_mappings>
    entry = _x86_os_cpu_lapic_mapping_find_slot(INVALID_CPU_ID);
    if(!entry)
    {
        DRV_ERR("CMD_SECAGENT_X86_INIT_COLLECT_LAPIC_ID_PERCPU error: No free slots for current CPU-LAPIC ID mapping!\n");
        return E_INTERNAL_ERROR;
    }

    entry->cpu_id = cur_cpu_id;
    entry->lapic_id = (lapic_id_t)cur_lapic_id;

	return OS_SUCCESS;
}