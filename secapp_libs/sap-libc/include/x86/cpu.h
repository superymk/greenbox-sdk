// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_X86_CPU_H
#define SECAPP_X86_CPU_H

#include <unistd.h>

#ifndef __ASSEMBLY__

/// @brief Save energy when waiting in a busy loop
static inline void cpu_relax(void) 
{
	// Busy-loop for a while, in order to allow the secure domain to receive and handle interrupts.
	usleep(100);
	asm volatile ("pause");
}


#endif // __ASSEMBLY__
#endif // SECAPP_X86_CPU_H