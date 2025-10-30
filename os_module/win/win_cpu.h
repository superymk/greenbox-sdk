// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_CPU_H
#define SECAGENT_WIN_CPU_H

#include <ntddk.h>
#include <wdf.h>
#include <sa_headers.h>

#ifndef __ASSEMBLY__

extern uint32_t win_read_processor_id(void);
extern process_id win_get_current_pid(void);

extern uint64_t win_read_msr(uint32_t msr);
extern void win_write_msr(uint32_t reg, uint32_t low, uint32_t high);

extern void win_atomic_increase(uint32_t *var);
extern void win_atomic_decrease(uint32_t* var);

extern void win_sos_spin_lock_init(volatile lock_t* lock);
extern void win_sos_spin_lock(volatile lock_t* lock);
extern bool win_sos_spin_trylock(volatile lock_t* lock);
extern void win_sos_spin_unlock(volatile lock_t* lock);

extern void win_udelay(uint32_t usec);
extern void win_msleep(uint32_t ms);

extern uint8_t win_inb(uint16_t port);
extern uint16_t win_inw(uint16_t port);
extern uint32_t win_inl(uint16_t port);
extern void win_outb(uint8_t value, uint16_t port);
extern void win_outw(uint16_t value, uint16_t port);
extern void win_outl(uint32_t value, uint16_t port);

/******* SecBase interfaces *******/
extern bool win_secbase_test_presence(void);
extern ulong_t win_hypercall_0arg(ulong_t fn);
extern ulong_t win_hypercall_1arg(ulong_t fn, ulong_t arg1);
extern ulong_t win_hypercall_2args(ulong_t fn, ulong_t arg1, ulong_t arg2);
extern ulong_t win_hypercall_3args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3);
extern ulong_t win_hypercall_4args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4);
extern ulong_t win_hypercall_ret1(ulong_t fn, ulong_t* arg1);
extern ulong_t win_hypercall_ret2(ulong_t fn, ulong_t* arg1, ulong_t* arg2);
extern ulong_t win_hypercall_ret3(ulong_t fn, ulong_t* arg1, ulong_t* arg2, ulong_t* arg3);
extern ulong_t win_hypercall_ret4(ulong_t fn, ulong_t* arg1, ulong_t* arg2, ulong_t* arg3, ulong_t* arg4);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_CPU_H