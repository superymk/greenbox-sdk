// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "win_cpu.h"
#include <intrin.h>
#include <sa_headers.h>

extern ulong_t Intel_VMCALL_ret4(ulong_t fn, ulong_t *ret_cx, ulong_t *ret_dx, ulong_t *ret_si, ulong_t *ret_di);

void win_udelay(uint32_t usec)
{
    KeStallExecutionProcessor((ULONG)usec);
}

void win_msleep(uint32_t ms)
{
    win_udelay(1000 * ms);
}

uint32_t win_read_processor_id(void)
{
    return (uint32_t)KeGetCurrentProcessorNumber();
}

process_id win_get_current_pid(void)
{
    return (process_id)PsGetCurrentProcessId();
}

uint64_t win_read_msr(uint32_t msr)
{
    return (uint64_t)__readmsr(msr);
}

void win_write_msr(uint32_t reg, uint32_t low, uint32_t high)
{
    uint64_t val = (((uint64_t)high << 32ULL) | low);
    __writemsr(reg, val);
}

void win_atomic_increase(uint32_t *var)
{
    InterlockedExchangeAdd((volatile long *)var, 1);
    os->cache_flush_range((vaddr_t)var, sizeof(uint32_t));
}

void win_atomic_decrease(uint32_t *var)
{
    InterlockedExchangeAdd((volatile long *)var, -1);
    os->cache_flush_range((vaddr_t)var, sizeof(uint32_t));
}

// void win_native_spin_lock_init(volatile os_native_lock_t *lock)
// {
//     if (!lock)
//     {
//         DRV_ERR("Invalid spin lock to init\n");
//         return;
//     }

//     KeInitializeSpinLock(&((os_native_lock_t *)lock)->lock);
// }

// void win_native_spin_lock(volatile os_native_lock_t *lock)
// {
//     if (!lock)
//     {
//         DRV_ERR("Invalid spin lock to acquire\n");
//         return;
//     }

//     KeAcquireSpinLock(&((os_native_lock_t *)lock)->lock, &lock->irql);
// }

// bool win_native_spin_trylock(volatile os_native_lock_t *lock)
// {
//     if (!lock)
//     {
//         return false;
//     }

//     if (!KeTestSpinLock(&((os_native_lock_t *)lock)->lock))
//     {
//         return false;
//     }

//     KeAcquireSpinLock(&((os_native_lock_t *)lock)->lock, &lock->irql);
//     return true;
// }

// void win_native_spin_unlock(volatile os_native_lock_t *lock)
// {
//     if (!lock)
//     {
//         DRV_ERR("Invalid spin lock to release\n");
//         return;
//     }

//     KeReleaseSpinLock(&((os_native_lock_t *)lock)->lock, lock->irql);
// }

void win_sos_spin_lock_init(volatile lock_t *lock)
{
    *lock = 1;
}

bool win_sos_spin_trylock(volatile lock_t* lock)
{
    /* Returns previous bit value and clears it */
    return _interlockedbittestandreset(l, 0) ? 1 : 0;
}

void win_sos_spin_lock(volatile lock_t* lock)
{
    for (;;) {
        if (_bittest((const long*)lock, 0)) {           /* looks free */
            if (win_sos_spin_trylock(lock))
                break;
        }
        do {
            YieldProcessor();                        /* PAUSE */
        } while (!_bittest((const long*)lock, 0));
    }
    KeMemoryBarrier(); /* pair with subsequent critical-section loads */
}

void win_sos_spin_unlock(volatile lock_t* lock)
{
    KeMemoryBarrier();                 /* publish writes before unlock */
    (void)_interlockedbittestandset(l, 0); /* set bit0 back to 1 */
}

uint8_t win_inb(uint16_t port)
{
    return (uint8_t)__inbyte(port);
}

uint16_t win_inw(uint16_t port)
{
    return (uint16_t)__inword(port);
}

uint32_t win_inl(uint16_t port)
{
    return (uint32_t)__indword(port);
}

void win_outb(uint8_t value, uint16_t port)
{
    __outbyte(port, value);
}

void win_outw(uint16_t value, uint16_t port)
{
    __outword(port, value);
}

void win_outl(uint32_t value, uint16_t port)
{
    __outdword(port, value);
}

bool win_secbase_test_presence(void)
{
    int cpuInfo[4];
    uint32_t resp = 0;

    __cpuid(cpuInfo, SECBASE_PRESENCE_REQ);
    resp = (uint32_t)cpuInfo[3];
    if (resp == SECBASE_PRESENCE_RESP)
        return true;

    return false;
}

ulong_t win_hypercall_0arg(ulong_t fn)
{
    ulong_t temp = 0;

    ulong_t result;
    // os->int3_debug_break();
    result = Intel_VMCALL_ret4(fn, &temp, &temp, &temp, &temp);
    // os->int3_debug_break();
    return result;
}

ulong_t win_hypercall_1arg(ulong_t fn, ulong_t arg1)
{
    ulong_t temp = 0;
    ulong_t in_arg1 = arg1; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    return Intel_VMCALL_ret4(fn, &in_arg1, &temp, &temp, &temp);
}

ulong_t win_hypercall_2args(ulong_t fn, ulong_t arg1, ulong_t arg2)
{
    ulong_t temp = 0;
    ulong_t in_arg1 = arg1; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    ulong_t in_arg2 = arg2; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    return Intel_VMCALL_ret4(fn, &in_arg1, &in_arg2, &temp, &temp);
}

ulong_t win_hypercall_3args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3)
{
    ulong_t temp = 0;
    ulong_t in_arg1 = arg1; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    ulong_t in_arg2 = arg2; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    ulong_t in_arg3 = arg3; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    return Intel_VMCALL_ret4(fn, &in_arg1, &in_arg2, &in_arg3, &temp);
}

ulong_t win_hypercall_4args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4)
{
    ulong_t in_arg1 = arg1; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    ulong_t in_arg2 = arg2; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    ulong_t in_arg3 = arg3; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    ulong_t in_arg4 = arg4; // We must copy the arguments, because Intel_VMCALL_ret4 overwrites input parameters except <fn>
    return Intel_VMCALL_ret4(fn, &in_arg1, &in_arg2, &in_arg3, &in_arg4);
}

extern void Int3(void);

ulong_t win_hypercall_ret1(ulong_t fn, ulong_t *arg1)
{
    ulong_t reserved = 0;

    // Int3();
    return Intel_VMCALL_ret4(fn, arg1, &reserved, &reserved, &reserved);
}

ulong_t win_hypercall_ret2(ulong_t fn, ulong_t *arg1, ulong_t *arg2)
{
    ulong_t reserved = 0;

    return Intel_VMCALL_ret4(fn, arg1, arg2, &reserved, &reserved);
}

ulong_t win_hypercall_ret3(ulong_t fn, ulong_t *arg1, ulong_t *arg2, ulong_t* arg3)
{
    ulong_t reserved = 0;

    return Intel_VMCALL_ret4(fn, arg1, arg2, arg3, &reserved);
}

ulong_t win_hypercall_ret4(ulong_t fn, ulong_t *arg1, ulong_t *arg2, ulong_t* arg3, ulong_t* arg4)
{
    return Intel_VMCALL_ret4(fn, arg1, arg2, arg3, arg4);
}