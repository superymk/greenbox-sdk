#ifndef SECAGENT_LINUX_CPU_H
#define SECAGENT_LINUX_CPU_H

#include <sa_headers.h>
#include <asm/cpufeature.h>

static inline void cpuid_eaxedx(uint32_t index, uint32_t *eax, uint32_t *edx)
{
	uint32_t ebx, ecx;
    cpuid(index, eax, &ebx, &ecx, edx);
}

extern uint64_t linux_read_msr(u32 msr);
extern void linux_write_msr(uint32_t reg, uint32_t low, uint32_t high);

extern void linux_atomic_increase(uint32_t *var);
extern void linux_atomic_decrease(uint32_t *var);

extern void linux_spin_lock_init(volatile lock_t* lock);
extern void linux_spin_lock(volatile lock_t* lock);
extern bool linux_spin_trylock(volatile lock_t* lock);
extern void linux_spin_unlock(volatile lock_t* lock);

extern void linux_udelay(uint32_t usec);
extern void linux_msleep(uint32_t msec);

extern uint8_t linux_inb(uint16_t port);
extern uint16_t linux_inw(uint16_t port);
extern uint32_t linux_inl(uint16_t port);
extern void linux_outb(uint8_t value, uint16_t port);
extern void linux_outw(uint16_t value, uint16_t port);
extern void linux_outl(uint32_t value, uint16_t port);

/******* SecBase interfaces *******/
extern bool linux_secbase_test_presence(void);
extern ulong_t linux_hypercall_0arg(ulong_t fn);
extern ulong_t linux_hypercall_1arg(ulong_t fn, ulong_t arg1);
extern ulong_t linux_hypercall_2args(ulong_t fn, ulong_t arg1, ulong_t arg2);
extern ulong_t linux_hypercall_3args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3);
extern ulong_t linux_hypercall_4args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4);
extern ulong_t linux_hypercall_ret1(ulong_t fn, ulong_t* arg1);

#endif // SECAGENT_LINUX_CPU_H