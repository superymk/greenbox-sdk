// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

#ifdef __amd64__

paddr_t lapic_get_base(void)
{
    uint32_t eax = 0, edx = 0;
    uint64_t msr = 0;

    msr = os->read_msr(IA32_APIC_BASE_MSR);
    eax = msr & 0xFFFFFFFF;
    edx = msr >> 32;

    return (eax & PAGE_MASK_4K) | (((ulong_t)(edx & 0x0fUL)) << 32ULL);
}

#endif // __amd64__