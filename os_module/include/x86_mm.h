// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECOS_LOADER_X86_MM_H
#define SECOS_LOADER_X86_MM_H

#ifdef __i386__
    #define PD_SHIFT        (22)      // offset of PDX in a linear address
#elif defined(__amd64__)
    #define PD_SHIFT        (21)      // offset of PDX in a linear address
#else /* !defined(__i386__) && !defined(__amd64__) */
    #error "Unsupported Arch"
#endif /* !defined(__i386__) && !defined(__amd64__) */

#define X86_LARGE_PAGE_SIZE		(_AC(1, UL) << PD_SHIFT)

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PWT         0x008   // Write-Through
#define PTE_PCD         0x010   // Cache-Disable
#define PTE_A           0x020   // Accessed
#define PTE_D           0x040   // Dirty
#define PTE_PS          0x080   // Page Size
#define PTE_MBZ         0x180   // Bits must be zero

#endif //  SECOS_LOADER_X86_MM_H