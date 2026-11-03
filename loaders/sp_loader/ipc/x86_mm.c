// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>

/* Perform CPUID(EAX=op, ECX=0), write results to *eax, *ebx, *ecx, *edx */
#define cpuid(op, eax, ebx, ecx, edx)                                                 \
    ({                                                                                \
        __asm__ __volatile__("cpuid"                                                  \
                             : "=a"(*(eax)), "=b"(*(ebx)), "=c"(*(ecx)), "=d"(*(edx)) \
                             : "0"(op), "2"(0));                                      \
    })

static inline unsigned int cpuid_eax(unsigned int op)
{
    unsigned int eax, ebx, ecx, edx;

    cpuid(op, &eax, &ebx, &ecx, &edx);

    return eax;
}

static inline unsigned int cpuid_ebx(unsigned int op)
{
    unsigned int eax, ebx, ecx, edx;

    cpuid(op, &eax, &ebx, &ecx, &edx);

    return ebx;
}

static inline unsigned int cpuid_ecx(unsigned int op)
{
    unsigned int eax, ebx, ecx, edx;

    cpuid(op, &eax, &ebx, &ecx, &edx);

    return ecx;
}

static inline unsigned int cpuid_edx(unsigned int op)
{
    unsigned int eax, ebx, ecx, edx;

    cpuid(op, &eax, &ebx, &ecx, &edx);

    return edx;
}

#define CPUID_EXTEND_FUNCS_QUERY (0x80000000)

/// @brief Read from CPUID extended function, which has CPUID >= 0x80000000.
/// @param op
/// @param eax
/// @return Return true on success.
///			Return false on errors.
static inline bool cpuid_ext_funcs_eax(unsigned int op, unsigned int *eax)
{
    uint32_t val = 0;

    val = cpuid_eax(CPUID_EXTEND_FUNCS_QUERY);

    if (val >= op)
    {
        *eax = cpuid_eax(op);
        return true;
    }

    // Invalid op
    return false;
}

/// @brief Read from CPUID extended function, which has CPUID >= 0x80000000.
/// @param op
/// @param eax
/// @return Return true on success.
///			Return false on errors.
static inline bool cpuid_ext_funcs_edx(unsigned int op, unsigned int *edx)
{
    uint32_t val = 0;

    val = cpuid_eax(CPUID_EXTEND_FUNCS_QUERY);

    if (val >= op)
    {
        *edx = cpuid_edx(op);
        return true;
    }

    // Invalid op
    return false;
}

static inline void clflush_ins(volatile void *p)
{
    asm volatile("clflush (%0)" ::"r"(p));
}

#define CPUID_FLAG_CLFL 0x80000 /* CLFLUSH - fixme? */

bool cache_flush_range(vaddr_t vaddr, size_t size)
{
    size_t clflush_size;
    vaddr_t p, vend;
    bool cpu_support_clflush = false;

    // Check: The current CPU must support CLFLUSH
    cpu_support_clflush = (cpuid_edx(1) & CPUID_FLAG_CLFL) ? true : false;
    if (!cpu_support_clflush)
        return false;

    clflush_size = ((cpuid_ebx(1) >> 8) & 0xff) << 3;
    p = (vaddr_t)vaddr & ~(clflush_size - 1);
    vend = vaddr + size;

    // Check: No overflow of (vaddr + size)
    // [NOTE] <p> also tells us the start of the aligned cache line
    if (p >= vend)
        return false;

    for (; p < vend; p += clflush_size)
        clflush_ins((void *)p);

    return true;
}