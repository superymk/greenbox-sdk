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

void win_spin_lock_init(volatile lock_t *lock)
{
    if (!lock)
    {
        DRV_ERR("Invalid spin lock to init\n");
        return;
    }

    KeInitializeSpinLock(&((lock_t *)lock)->lock);
}

void win_spin_lock(volatile lock_t *lock)
{
    if (!lock)
    {
        DRV_ERR("Invalid spin lock to acquire\n");
        return;
    }

    KeAcquireSpinLock(&((lock_t *)lock)->lock, &lock->irql);
}

bool win_spin_trylock(volatile lock_t *lock)
{
    if (!lock)
    {
        return false;
    }

    if (!KeTestSpinLock(&((lock_t *)lock)->lock))
    {
        return false;
    }

    KeAcquireSpinLock(&((lock_t *)lock)->lock, &lock->irql);
    return true;
}

void win_spin_unlock(volatile lock_t *lock)
{
    if (!lock)
    {
        DRV_ERR("Invalid spin lock to release\n");
        return;
    }

    KeReleaseSpinLock(&((lock_t *)lock)->lock, lock->irql);
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