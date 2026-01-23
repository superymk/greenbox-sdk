// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_CPU_H
#define SECAGENT_WIN_CPU_H

#include <ntddk.h>
#include <wdf.h>
#include <sa_headers.h>

#ifndef __ASSEMBLY__

/// @brief Get current processor ID
/// @return Processor ID
extern uint32_t win_read_processor_id(void);

/// @brief Get current process ID
/// @return Process ID
extern process_id win_get_current_pid(void);

/// @brief Read Model-Specific Register (MSR)
/// @param msr MSR address
/// @return MSR value
extern uint64_t win_read_msr(uint32_t msr);

/// @brief Write Model-Specific Register (MSR)
/// @param reg MSR address
/// @param low Low 32 bits of value
/// @param high High 32 bits of value
extern void win_write_msr(uint32_t reg, uint32_t low, uint32_t high);

/// @brief Atomically increment a variable
/// @param var Pointer to variable to increment
extern void win_atomic_increase(uint32_t *var);

/// @brief Atomically decrement a variable
/// @param var Pointer to variable to decrement
extern void win_atomic_decrease(uint32_t *var);

/// @brief Initialize a spinlock
/// @param lock Pointer to lock structure
extern void win_spin_lock_init(volatile lock_t *lock);

/// @brief Acquire a spinlock
/// @param lock Pointer to lock structure
extern void win_spin_lock(volatile lock_t *lock);

/// @brief Try to acquire a spinlock (non-blocking)
/// @param lock Pointer to lock structure
/// @return true if acquired, false otherwise
extern bool win_spin_trylock(volatile lock_t *lock);

/// @brief Release a spinlock
/// @param lock Pointer to lock structure
extern void win_spin_unlock(volatile lock_t *lock);

/// @brief Sleep for specified microseconds
/// @param usec Microseconds to sleep
extern void win_udelay(uint32_t usec);

/// @brief Sleep for specified milliseconds
/// @param ms Milliseconds to sleep
extern void win_msleep(uint32_t ms);

/// @brief Read byte from I/O port
/// @param port I/O port address
/// @return Byte read from port
extern uint8_t win_inb(uint16_t port);

/// @brief Read word from I/O port
/// @param port I/O port address
/// @return Word read from port
extern uint16_t win_inw(uint16_t port);

/// @brief Read double word from I/O port
/// @param port I/O port address
/// @return Double word read from port
extern uint32_t win_inl(uint16_t port);

/// @brief Write byte to I/O port
/// @param value Value to write
/// @param port I/O port address
extern void win_outb(uint8_t value, uint16_t port);

/// @brief Write word to I/O port
/// @param value Value to write
/// @param port I/O port address
extern void win_outw(uint16_t value, uint16_t port);

/// @brief Write double word to I/O port
/// @param value Value to write
/// @param port I/O port address
extern void win_outl(uint32_t value, uint16_t port);

/******* SecBase interfaces *******/

/// @brief Test if SecBase (SecOS) is present
/// @return true if SecBase is present, false otherwise
extern bool win_secbase_test_presence(void);

/// @brief Make VMCALL hypercall with no arguments
/// @param fn Hypercall function code
/// @return Return value from hypercall
extern ulong_t win_hypercall_0arg(ulong_t fn);

/// @brief Make VMCALL hypercall with one argument
/// @param fn Hypercall function code
/// @param arg1 First argument
/// @return Return value from hypercall
extern ulong_t win_hypercall_1arg(ulong_t fn, ulong_t arg1);

/// @brief Make VMCALL hypercall with two arguments
/// @param fn Hypercall function code
/// @param arg1 First argument
/// @param arg2 Second argument
/// @return Return value from hypercall
extern ulong_t win_hypercall_2args(ulong_t fn, ulong_t arg1, ulong_t arg2);

/// @brief Make VMCALL hypercall with three arguments
/// @param fn Hypercall function code
/// @param arg1 First argument
/// @param arg2 Second argument
/// @param arg3 Third argument
/// @return Return value from hypercall
extern ulong_t win_hypercall_3args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3);

/// @brief Make VMCALL hypercall with four arguments
/// @param fn Hypercall function code
/// @param arg1 First argument
/// @param arg2 Second argument
/// @param arg3 Third argument
/// @param arg4 Fourth argument
/// @return Return value from hypercall
extern ulong_t win_hypercall_4args(ulong_t fn, ulong_t arg1, ulong_t arg2, ulong_t arg3, ulong_t arg4);

/// @brief Make VMCALL hypercall with one return value
/// @param fn Hypercall function code
/// @param arg1 Pointer to receive first return value
/// @return Return value from hypercall
extern ulong_t win_hypercall_ret1(ulong_t fn, ulong_t *arg1);

/// @brief Make VMCALL hypercall with two return values
/// @param fn Hypercall function code
/// @param arg1 Pointer to receive first return value
/// @param arg2 Pointer to receive second return value
/// @return Return value from hypercall
extern ulong_t win_hypercall_ret2(ulong_t fn, ulong_t *arg1, ulong_t *arg2);

/// @brief Make VMCALL hypercall with three return values
/// @param fn Hypercall function code
/// @param arg1 Pointer to receive first return value
/// @param arg2 Pointer to receive second return value
/// @param arg3 Pointer to receive third return value
/// @return Return value from hypercall
extern ulong_t win_hypercall_ret3(ulong_t fn, ulong_t *arg1, ulong_t *arg2, ulong_t *arg3);

/// @brief Make VMCALL hypercall with four return values
/// @param fn Hypercall function code
/// @param arg1 Pointer to receive first return value
/// @param arg2 Pointer to receive second return value
/// @param arg3 Pointer to receive third return value
/// @param arg4 Pointer to receive fourth return value
/// @return Return value from hypercall
extern ulong_t win_hypercall_ret4(ulong_t fn, ulong_t *arg1, ulong_t *arg2, ulong_t *arg3, ulong_t *arg4);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_CPU_H