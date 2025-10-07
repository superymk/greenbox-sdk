// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// #include <include/linux/delay.h> /* Needed for udelay */
#include <linux/delay.h>
#include <linux/spinlock.h>
#include "linux_cpu.h"

uint64_t linux_read_msr(u32 msr)
{
    u32 eax, edx;
    __asm__("rdmsr"
            : "=a"(eax), "=d"(edx)
            : "c"(msr));

    return ((uint64_t)edx << 32) | eax;
}

void linux_write_msr(uint32_t reg, uint32_t low, uint32_t high)
{
    wrmsr(reg, low, high);
}

void linux_atomic_increase(uint32_t *var)
{
    __asm __volatile(LOCK_PREFIX "incl %0"
                     : "=m"(*var)
                     : "m"(*var));
    os->cache_flush_range((vaddr_t)var, sizeof(uint32_t));
}

void linux_atomic_decrease(uint32_t *var)
{
    __asm __volatile(LOCK_PREFIX "decl %0"
                     : "=m"(*var)
                     : "m"(*var));
    os->cache_flush_range((vaddr_t)var, sizeof(uint32_t));
}

void linux_spin_lock_init(volatile lock_t *lock)
{
    spin_lock_init((lock_t *)lock);
}

void linux_spin_lock(volatile lock_t *lock)
{
    spin_lock_irq((lock_t *)lock);
}

bool linux_spin_trylock(volatile lock_t *lock)
{
    if (spin_trylock((lock_t *)lock))
        return true;
    else
        return false;
}

void linux_spin_unlock(volatile lock_t *lock)
{
    spin_unlock_irq((lock_t *)lock);
}

// void linux_spin_lock(volatile lock_t* lock)
// {
// 	asm volatile(
// 		"xorl %%ecx, %%ecx;"
// 		"incl %%ecx;"
// 		"mov %0, %%ebx;"    // lock_addr
// 	"1: ;"
// 	    "xorl %%eax, %%eax;"
// 	    "lock cmpxchgl %%ecx, (%%ebx);"
// 	    "pause;"
// 	    "jnz 1b;"::"r"(lock):"eax", "ebx", "ecx");
// }

// void linux_spin_unlock(volatile lock_t* lock)
// {
//     asm volatile (""); // acts as a memory barrier.
//     *lock = 0;
// }

void linux_udelay(uint32_t usec)
{
    udelay(usec);
}

void linux_msleep(uint32_t ms)
{
    // [TODO][Issue 87] Using <msleep> may cause SecAgent hang when SecOS is running on a processor.
    // msleep(ms);
    mdelay(ms);
}

uint8_t linux_inb(uint16_t port)
{
    uint8_t value;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(value) : "Nd"(port));
    return value;
}

uint16_t linux_inw(uint16_t port)
{
    uint16_t value;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(value) : "Nd"(port));
    return value;
}

uint32_t linux_inl(uint16_t port)
{
    uint32_t value;
    __asm__ __volatile__("inl %w1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void linux_outb(uint8_t value, uint16_t port)
{
    __asm__ __volatile__("outb %b0, %w1" : : "a"(value), "Nd"(port));
}

void linux_outw(uint16_t value, uint16_t port)
{
    __asm__ __volatile__("outw %w0, %w1" : : "a"(value), "Nd"(port));
}

void linux_outl(uint32_t value, uint16_t port)
{
    __asm__ __volatile__("outl %0, %w1" : : "a"(value), "Nd"(port));
}

/******* SecBase interfaces *******/
#define CPU_VENDOR_INTEL 0xAB
#define CPU_VENDOR_AMD 0xCD
#define CPU_VENDOR_UNKNOWN 0x00

#define AMD_STRING_DWORD1 0x68747541
#define AMD_STRING_DWORD2 0x69746E65
#define AMD_STRING_DWORD3 0x444D4163

#define INTEL_STRING_DWORD1 0x756E6547
#define INTEL_STRING_DWORD2 0x49656E69
#define INTEL_STRING_DWORD3 0x6C65746E

static uint32_t cpu_vendor = CPU_VENDOR_UNKNOWN;

static inline uint32_t _get_cpu_vendor(void)
{
    uint32_t dummy;
    uint32_t vendor_dword1, vendor_dword2, vendor_dword3;

    cpuid(0, &dummy, &vendor_dword1, &vendor_dword3, &vendor_dword2);
    if (vendor_dword1 == AMD_STRING_DWORD1 && vendor_dword2 == AMD_STRING_DWORD2 && vendor_dword3 == AMD_STRING_DWORD3)
        return CPU_VENDOR_AMD;
    else if (vendor_dword1 == INTEL_STRING_DWORD1 && vendor_dword2 == INTEL_STRING_DWORD2 && vendor_dword3 == INTEL_STRING_DWORD3)
        return CPU_VENDOR_INTEL;
    else
        return CPU_VENDOR_UNKNOWN;

    return 0; /* never reached */
}

static inline ulong_t linux_hypercall(ulong_t eax,
                                      ulong_t ecx, ulong_t edx, ulong_t esi, ulong_t edi,
                                      ulong_t *out_esi, ulong_t *out_edi)
{
    // Flush cache first to ensure parameters to SecBase are flushed to memory
    wbinvd();

    // Decide and cache the info of CPU vendor, so we do not need to repeat to issue the expensive CPUID instruction.
    if (cpu_vendor == CPU_VENDOR_UNKNOWN)
    {
        cpu_vendor = _get_cpu_vendor();
    }

    // Issue a hypercall
    switch (cpu_vendor)
    {
    case CPU_VENDOR_INTEL:
        __asm__ __volatile__(
            "vmcall\n\t"
            : "=a"(eax), "=S"(*out_esi), "=D"(*out_edi)
            : "a"(eax), "c"(ecx), "d"(edx),
              "S"(esi), "D"(edi));
        break;
    case CPU_VENDOR_AMD:
        __asm__ __volatile__(
            "vmmcall\n\t"
            : "=a"(eax), "=S"(*out_esi), "=D"(*out_edi)
            : "a"(eax), "c"(ecx), "d"(edx),
              "S"(esi), "D"(edi));
        break;
    default:
        eax = -1;
    }

    return eax;
}

static inline ulong_t linux_hypercall_ret4(ulong_t arg_ax, ulong_t *arg_cx, ulong_t *arg_dx,
                                           ulong_t *arg_si, ulong_t *arg_di)
{
    // Flush cache first to ensure parameters to SecBase are flushed to memory
    wbinvd();

    // Decide and cache the info of CPU vendor, so we do not need to repeat to issue the expensive CPUID instruction.
    if (cpu_vendor == CPU_VENDOR_UNKNOWN)
    {
        cpu_vendor = _get_cpu_vendor();
    }

    switch (cpu_vendor)
    {
    case CPU_VENDOR_INTEL:
        __asm__ __volatile__(
            "vmcall\n\t"
            : "=a"(arg_ax), "=c"(*arg_cx), "=d"(*arg_dx),
              "=S"(*arg_si), "=D"(*arg_di)
            : "a"(arg_ax), "c"(*arg_cx), "d"(*arg_dx),
              "S"(*arg_si), "D"(*arg_di));
        break;
    case CPU_VENDOR_AMD:
        __asm__ __volatile__(
            "vmmcall\n\t"
            : "=a"(arg_ax), "=c"(*arg_cx), "=d"(*arg_dx),
              "=S"(*arg_si), "=D"(*arg_di)
            : "a"(arg_ax), "c"(*arg_cx), "d"(*arg_dx),
              "S"(*arg_si), "D"(*arg_di));
        break;
    default:
        arg_ax = -1;
    }

    return arg_ax;
}

bool linux_secbase_test_presence(void)
{
    uint32_t resp = 0;

    resp = cpuid_edx(SECBASE_PRESENCE_REQ);
    if (resp == SECBASE_PRESENCE_RESP)
        return true;

    return false;
}

ulong_t linux_hypercall_0arg(ulong_t fn)
{
    ulong_t esi = 0, edi = 0;
    return linux_hypercall(fn, 0, 0, 0, 0, &esi, &edi);
}

ulong_t linux_hypercall_1arg(ulong_t fn, ulong_t arg1)
{
    ulong_t esi = 0, edi = 0;
    return linux_hypercall(fn, arg1, 0, 0, 0, &esi, &edi);
}

ulong_t linux_hypercall_2args(ulong_t fn, ulong_t arg1, ulong_t arg2)
{
    ulong_t esi = 0, edi = 0;
    return linux_hypercall(fn, arg1, arg2, 0, 0, &esi, &edi);
}

ulong_t linux_hypercall_3args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3)
{
    ulong_t esi = 0, edi = 0;
    return linux_hypercall(fn, arg1, arg2, arg3, 0, &esi, &edi);
}

ulong_t linux_hypercall_4args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4)
{
    ulong_t esi = 0, edi = 0;
    return linux_hypercall(fn, arg1, arg2, arg3, arg4, &esi, &edi);
}

ulong_t linux_hypercall_ret1(ulong_t fn, ulong_t *arg1)
{
    ulong_t reserved = 0;

    return linux_hypercall_ret4(fn, arg1, &reserved, &reserved, &reserved);
}
