// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_LINUX_CPU_H
#define SECAGENT_LINUX_CPU_H

#include <sa_headers.h>
#include <asm/cpufeature.h>

/// @brief Read CPUID EAX/EDX for <index>
/// @param index CPUID leaf
/// @param eax Output EAX
/// @param edx Output EDX
static inline void cpuid_eaxedx(uint32_t index, uint32_t *eax, uint32_t *edx)
{
	uint32_t ebx, ecx;
    cpuid(index, eax, &ebx, &ecx, edx);
}

/// @brief Read MSR
/// @param msr MSR index
/// @return MSR value
extern uint64_t linux_read_msr(uint32_t msr);

/// @brief Write MSR
/// @param reg MSR index
/// @param low Low 32 bits
/// @param high High 32 bits
extern void linux_write_msr(uint32_t reg, uint32_t low, uint32_t high);

/// @brief Atomically increase <var>
/// @param var Counter pointer
extern void linux_atomic_increase(uint32_t *var);

/// @brief Atomically decrease <var>
/// @param var Counter pointer
extern void linux_atomic_decrease(uint32_t *var);

/// @brief Initialize spinlock
/// @param lock Spinlock
extern void linux_spin_lock_init(volatile lock_t* lock);

/// @brief Acquire spinlock
/// @param lock Spinlock
extern void linux_spin_lock(volatile lock_t* lock);

/// @brief Try to acquire spinlock
/// @param lock Spinlock
/// @return true on success, false otherwise
extern bool linux_spin_trylock(volatile lock_t* lock);

/// @brief Release spinlock
/// @param lock Spinlock
extern void linux_spin_unlock(volatile lock_t* lock);

/// @brief Busy-wait delay
/// @param usec Microseconds
extern void linux_udelay(uint32_t usec);

/// @brief Millisecond sleep
/// @param msec Milliseconds
extern void linux_msleep(uint32_t msec);

/// @brief Read byte from I/O port
/// @param port I/O port
/// @return Value
extern uint8_t linux_inb(uint16_t port);

/// @brief Read word from I/O port
/// @param port I/O port
/// @return Value
extern uint16_t linux_inw(uint16_t port);

/// @brief Read dword from I/O port
/// @param port I/O port
/// @return Value
extern uint32_t linux_inl(uint16_t port);

/// @brief Write byte to I/O port
/// @param value Value
/// @param port I/O port
extern void linux_outb(uint8_t value, uint16_t port);

/// @brief Write word to I/O port
/// @param value Value
/// @param port I/O port
extern void linux_outw(uint16_t value, uint16_t port);

/// @brief Write dword to I/O port
/// @param value Value
/// @param port I/O port
extern void linux_outl(uint32_t value, uint16_t port);

/********* SecBase interfaces *********/

/// @brief Test SecBase presence
/// @return true if present, false otherwise
extern bool linux_secbase_test_presence(void);

/// @brief Hypercall with 0 args
/// @param fn Hypercall ID
/// @return Hypercall result
extern ulong_t linux_hypercall_0arg(ulong_t fn);

/// @brief Hypercall with 1 arg
/// @param fn Hypercall ID
/// @param arg1 Argument 1
/// @return Hypercall result
extern ulong_t linux_hypercall_1arg(ulong_t fn, ulong_t arg1);

/// @brief Hypercall with 2 args
/// @param fn Hypercall ID
/// @param arg1 Argument 1
/// @param arg2 Argument 2
/// @return Hypercall result
extern ulong_t linux_hypercall_2args(ulong_t fn, ulong_t arg1, ulong_t arg2);

/// @brief Hypercall with 3 args
/// @param fn Hypercall ID
/// @param arg1 Argument 1
/// @param arg2 Argument 2
/// @param arg3 Argument 3
/// @return Hypercall result
extern ulong_t linux_hypercall_3args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3);

/// @brief Hypercall with 4 args
/// @param fn Hypercall ID
/// @param arg1 Argument 1
/// @param arg2 Argument 2
/// @param arg3 Argument 3
/// @param arg4 Argument 4
/// @return Hypercall result
extern ulong_t linux_hypercall_4args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4);

/// @brief Hypercall returning 1 value
/// @param fn Hypercall ID
/// @param arg1 Output arg1
/// @return Hypercall result
extern ulong_t linux_hypercall_ret1(ulong_t fn, ulong_t* arg1);

/// @brief Hypercall returning 2 values
/// @param fn Hypercall ID
/// @param arg1 Output arg1
/// @param arg2 Output arg2
/// @return Hypercall result
extern ulong_t linux_hypercall_ret2(ulong_t fn, ulong_t* arg1, ulong_t* arg2);

/// @brief Hypercall returning 3 values
/// @param fn Hypercall ID
/// @param arg1 Output arg1
/// @param arg2 Output arg2
/// @param arg3 Output arg3
/// @return Hypercall result
extern ulong_t linux_hypercall_ret3(ulong_t fn, ulong_t* arg1, ulong_t* arg2, ulong_t* arg3);

/// @brief Hypercall returning 4 values
/// @param fn Hypercall ID
/// @param arg1 Output arg1
/// @param arg2 Output arg2
/// @param arg3 Output arg3
/// @param arg4 Output arg4
/// @return Hypercall result
extern ulong_t linux_hypercall_ret4(ulong_t fn, ulong_t* arg1, ulong_t* arg2, ulong_t* arg3, ulong_t* arg4);

#endif // SECAGENT_LINUX_CPU_H