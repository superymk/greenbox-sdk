// SPDX-License-Identifier: Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/*
 * Public API Header — Limited License
 * Copyright (c) 2021 - 2026 Miao Yu, Virgil Gligor
 *
 * Permission is granted to use, copy, and redistribute this header file
 * verbatim for the sole purpose of compiling software that interfaces with
 * GreenBox via its public API, provided that:
 *   (1) this notice is retained in all copies; and
 *   (2) the file is not modified.
 *
 * Including or referencing this header, by itself, does not impose any
 * copyleft or source-disclosure obligations on your software. Rights to
 * the GreenBox implementation are governed by LICENSE-PROPRIETARY
 * (or GPL-2.0-only where applicable). No patent rights are granted by this
 * header. Trademarks are not licensed.
 *
 * For commercial licensing, contact: Miao Yu (superymk-cmu-edu)
 */

#ifndef VK_MM_DEFS_H
#define VK_MM_DEFS_H

//////////// Definitions of CPUs ////////////
#ifdef __i386__
    #include "arch/i386/mm_defs.h"
#elif defined(__amd64__)
    #include "arch/amd64/mm_defs.h"
#else
    #error "Unsupported Arch"
#endif

#define KB(x) ((UL(x)) << 10)
#define MB(x) ((UL(x)) << 20)
#define GB(x) ((ULL(x)) << 30)


#define PAGE_SHIFT_4K			12

#define PAGE_SIZE_4K			((1UL) << PAGE_SHIFT_4K)          // 0x(00000000)1000

#define PAGE_MASK_4K			(~(PAGE_SIZE_4K-1))                   // 0x(FFFFFFFF)FFFFF000
#define PAGE_FLAGS_MASK_4K		(~PAGE_MASK_4K)
#define PAGE_OFFSET_4K			(PAGE_FLAGS_MASK_4K)

#define IS_PAGE_ALIGNED_4K(v)    ((v & PAGE_MASK_4K) == v)

#define SIZE_ALIGN_4K(size)		((size) & PAGE_MASK_4K)
#define SIZE_ROUNDUP_4K(size)	(((size) + PAGE_SIZE_4K - 1) & PAGE_MASK_4K)
#define BYTES_TO_PAGE_4K(size)	(SIZE_ROUNDUP_4K(size) >> PAGE_SHIFT_4K)
#define PAGE_4K_TO_BYTES(pages)	((pages) << PAGE_SHIFT_4K)

#define ADDR_TO_PFN(addr)		((addr) >> PAGE_SHIFT_4K)

//////////// SecOS boot param layout ///////////
//! SOS_LOADER_* is initialized by the SecOS loader, and passes data to the SecOS.
//! The total size of this space must be less than 256KB
#define SOS_LOADER_PT_BASE				(0x0)				//! vaddr. of init Page table. 0x0
#define SOS_LOADER_PT_SIZE				(8 * PAGE_SIZE_4K)
#define SOS_LOADER_STACK_BASE			(SOS_LOADER_PT_BASE + SOS_LOADER_PT_SIZE)	//! vaddr of stack when the SecOS is called by the loader. 0x1000
#define SOS_LOADER_STACK_SIZE			(SECOS_STACK_SIZE) //! size of the stack when the SecOS is called by the loader		




//////////// SecOS kernel memory layout  ///////////
#define SOS_KERN_SPACE_SIZE				(ADDR_MAX - SOS_KERN_BASE + 1) //! The address space size of the SecOS kernel
#define SOS_INITDATA_BASE				(SOS_KERN_BASE) //! vaddr of boot parameters initialized by the loader, start at 0xC0000000
#define SOS_INITDATA_SIZE				(SOS_LOADER_STACK_BASE + SOS_LOADER_STACK_SIZE)
#define SOS_PARAM_PAGE_BASE				(SOS_INITDATA_BASE + SOS_INITDATA_SIZE) //! Start at SOS_INITDATA_BASE + 52KB
#define SOS_PARAM_PAGE_SIZE				(2 << PAGE_SHIFT_4K)
#define SOS_SIGNATURE_PAGE_BASE			(SOS_PARAM_PAGE_BASE + SOS_PARAM_PAGE_SIZE) //! Start at SOS_INITDATA_BASE + 52KB + 8KB
#define SOS_SIGNATURE_PAGE_SIZE			(1 << PAGE_SHIFT_4K)


#define SOS_COMM_OS_BASE				(SOS_SIGNATURE_PAGE_BASE + SOS_SIGNATURE_PAGE_SIZE) //! Start at SOS_INITDATA_BASE + 52KB + 8KB + 4KB
#define SOS_COMM_OS_MAX_SIZE			(MB(1) - SOS_INITDATA_SIZE - SOS_PARAM_PAGE_SIZE - SOS_SIGNATURE_PAGE_SIZE)
#define SOS_CODE_BASE					(SOS_COMM_OS_BASE + SOS_COMM_OS_MAX_SIZE) //! vaddr. of SecOS kernel code, start at SOS_INITDATA_BASE + 1MB
#define SOS_HEAP_BASE					(SOS_INITDATA_BASE + MB(4)) //! vaddr. of SecOS heap memory, start at SOS_INITDATA_BASE + 4MB
#define SOS_HEAP_SIZE					(VK_MEM_SIZE)
#define SOS_SHARED_MEM_BASE				(SOS_HEAP_BASE + SOS_HEAP_SIZE)	//! vaddr. of SecOS heap memory, start at SOS_INITDATA_BASE + HEAP_SIZE + 4MB  (SOS_INITDATA_BASE + 8MB)
#define SOS_SHARED_MEM_SIZE				(VK_SHAREMEM_SIZE)
#define SOS_MAPPABLE_SPACE_BASE			(SOS_SHARED_MEM_BASE + SOS_SHARED_MEM_SIZE)
#define SOS_MAPPABLE_SPACE_SIZE			(SOS_HIGHDEV_BASE - SOS_MAPPABLE_SPACE_BASE) //! Free vaddr space. From SOS_MAPPABLE_SPACE_BASE to SOS_HIGHDEV_PADDR_BASE
#define SOS_HIGHDEV_SIZE				((ADDR_MAX & PAGE_MASK_4K) - SOS_HIGHDEV_BASE + PAGE_SIZE_4K)

#define K_VIRT_TO_PHYS(vaddr)			(paddr_t)((vaddr) - SOS_INITDATA_BASE)
#define K_PHYS_TO_VIRT(paddr)			(vaddr_t)((paddr) + SOS_INITDATA_BASE)




//////////// SecProcess Memory Layout ///////////
#define SP_BASE_VADDR					0x0
#define SP_TOP_VADDR					SOS_KERN_BASE




//////////// Physical Memory Layout ///////////
#ifdef __i386__
    /// @brief SecProcesses' paddr region can only exist in this paddr range. In other words, all SecProcesses can only be 
    /// loaded in this paddr range of the secure domain.
    /// [SECURITY-NOTE] The paddr region [SOS_MAPPABLE_SPACE_PADDR_BASE, SOS_MAPPABLE_SPACE_PADDR_END) must be separate from
    /// [SPS_MEM_PADDR_BASE, SPS_MEM_PADDR_END). This is because SecProcesses' platform memory objects (platform_memobjs) 
    /// must be mapped in the first region only, whereas SecProcesses paddr memory objects (secdom_memobjs) must exist in 
    /// the second region only.
    #define SPS_MEM_PADDR_BASE                  (0x10000000)
    #define SPS_MEM_PADDR_END                   (0xC0000000)

    /// @brief spaddr mapped by SecOS can only be mapped to this paddr range.
    /// [SECURITY-NOTE] The paddr region [SOS_MAPPABLE_SPACE_PADDR_BASE, SOS_MAPPABLE_SPACE_PADDR_END) must be separate from
    /// [SPS_MEM_PADDR_BASE, SPS_MEM_PADDR_END). This is because SecProcesses' platform memory objects (platform_memobjs) 
    /// must be mapped in the first region only, whereas SecProcesses paddr memory objects (secdom_memobjs) must exist in 
    /// the second region only.
    #define SOS_MAPPABLE_SPACE_PADDR_BASE       (SPS_MEM_PADDR_END) 
    #define SOS_MAPPABLE_SPACE_PADDR_END        ((paddr_t)SOS_HIGHDEV_BASE) // The end of the paddr range cannot be overlapped with SOS_HIGHDEV_BASE,
                                                                // because SecOS creates flat mapping of SOS_HIGHDEV_BASE and above.

#elif defined(__amd64__)

    /// @brief SecProcesses' paddr region can only exist in this paddr range. In other words, all SecProcesses can only be 
    /// loaded in this paddr range of the secure domain.
    /// [SECURITY-NOTE] The paddr region [SOS_MAPPABLE_SPACE_PADDR_BASE, SOS_MAPPABLE_SPACE_PADDR_END) must be separate from
    /// [SPS_MEM_PADDR_BASE, SPS_MEM_PADDR_END). This is because SecProcesses' platform memory objects (platform_memobjs) 
    /// must be mapped in the first region only, whereas SecProcesses paddr memory objects (secdom_memobjs) must exist in 
    /// the second region only.
    #define SPS_MEM_PADDR_BASE                  (0x000100000000) // Start from 4GB to bypass the MMIO region in low-4GB.
    #define SPS_MEM_PADDR_END                   (0x000800000000)

    /// @brief spaddr mapped by SecOS can only be mapped to this paddr range.
    /// [SECURITY-NOTE] The paddr region [SOS_MAPPABLE_SPACE_PADDR_BASE, SOS_MAPPABLE_SPACE_PADDR_END) must be separate from
    /// [SPS_MEM_PADDR_BASE, SPS_MEM_PADDR_END). This is because SecProcesses' platform memory objects (platform_memobjs) 
    /// must be mapped in the first region only, whereas SecProcesses paddr memory objects (secdom_memobjs) must exist in 
    /// the second region only.
    #define SOS_MAPPABLE_SPACE_PADDR_BASE       (SPS_MEM_PADDR_END)

    /// @brief The end of the paddr range cannot be overlapped with SOS_HIGHDEV_BASE, because SecOS creates flat mapping 
    /// of SOS_HIGHDEV_BASE and above.
    /// [NOTE] The general MAXPHYADDR limit of Intel x64 CPUs is 36 if supports PAE, see SDM volume 3, Section 5.1.4.
    #define SOS_MAPPABLE_SPACE_PADDR_END        ((paddr_t)0x001000000000)

#endif // __i386__

#endif // VK_MM_DEFS_H