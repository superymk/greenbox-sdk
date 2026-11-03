// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_WIN_MEMORY_H
#define SECAGENT_WIN_MEMORY_H

#include "win_cpu.h"
#include <sa_headers.h>

#ifndef __ASSEMBLY__

/// @brief Allocate zeroed non-paged memory
/// @param size Size in bytes to allocate
/// @return Pointer to allocated memory, NULL on failure
extern void* win_mm_malloc_zeroed(size_t size);

/// @brief Free allocated memory
/// @param ptr Pointer to memory to free
extern void win_mm_free(void* ptr);

/// @brief Allocate memory pages
/// @param num_pages Number of pages to allocate
/// @param type Allocation type (ALLOC_ORDINARY_MEMORY or ALLOC_DMA_MEMORY)
/// @return Virtual address of allocated memory, INVALID_VADDR on failure
extern vaddr_t win_mm_alloc_pages(uint32_t num_pages, uint32_t type);

/// @brief Convert virtual address to physical address
/// @param vaddr Virtual address
/// @return Physical address, INVALID_PADDR on failure
extern paddr_t win_mm_vaddr_to_paddr(vaddr_t vaddr);

/// @brief Allocate physically contiguous memory pages
/// @param num_pages Number of pages to allocate
/// @param type Allocation type (ALLOC_ORDINARY_MEMORY or ALLOC_DMA_MEMORY)
/// @return Virtual address of allocated memory, INVALID_VADDR on failure
extern vaddr_t win_mm_get_physical_contiguous_pages(uint32_t num_pages, uint32_t type);

/// @brief Allocate physically contiguous non-cached memory pages
/// @param num_pages Number of pages to allocate
/// @param type Allocation type (unused, always uses ALLOC_DMA_MEMORY)
/// @return Virtual address of allocated memory, INVALID_VADDR on failure
extern vaddr_t win_mm_alloc_physical_contiguous_pages_nocache(uint32_t num_pages, uint32_t type);

/// @brief Free physically contiguous memory pages
/// @param vaddr Virtual address of memory to free
/// @param num_pages Number of pages to free
/// @param type Allocation type (ALLOC_ORDINARY_MEMORY or ALLOC_DMA_MEMORY)
extern void win_mm_free_physical_contiguous_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);

/// @brief Free allocated memory pages
/// @param vaddr Virtual address of memory to free
/// @param num_pages Number of pages to free
/// @param type Allocation type (ALLOC_ORDINARY_MEMORY or ALLOC_DMA_MEMORY)
extern void win_mm_free_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);

/// @brief Map physical address to kernel virtual address (non-cached)
/// @param paddr Physical address to map
/// @param size Size of region to map in bytes
/// @return Kernel virtual address, NULL on failure
extern void* win_ioremap_nocache(paddr_t paddr, size_t size);

/// @brief Unmap previously mapped I/O space
/// @param ptr Mapped virtual address
/// @param size Size of mapped region in bytes
extern void win_iounmap(void* ptr, size_t size);

/// @brief Set memory to a byte value
/// @param ptr Destination pointer
/// @param v Byte value
/// @param size Number of bytes
extern void win_memset(void* ptr, char v, size_t size);

/// @brief Copy memory
/// @param __to Destination pointer
/// @param __from Source pointer
/// @param __n Number of bytes
extern void win_memcpy(void *__to, const void *__from, size_t __n);

/// @brief Copy from kernel to user buffer
/// @param usr_dest User destination pointer
/// @param kern_src Kernel source pointer
/// @param size Number of bytes
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS win_copy_to_user(void* usr_dest, const void* kern_src, size_t size);

/// @brief Copy from user to kernel buffer
/// @param kern_dest Kernel destination pointer
/// @param usr_src User source pointer
/// @param size Number of bytes
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS win_copy_from_user(void* kern_dest, const void* usr_src, size_t size);

/// @brief Memory fence
extern void win_mm_fence(void);

/// @brief Write back and invalidate cache
extern void win_wbinvd(void);

/// @brief Flush cache lines over a virtual address range
/// @param vaddr Start virtual address
/// @param size Size in bytes
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS win_cache_flush_range(vaddr_t vaddr, size_t size);

/// @brief Check current process page permissions for a user range
/// @param uvaddr User virtual address
/// @param size Size in bytes
/// @param out_allowed_read_write Output flag for RW permission
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS win_mm_check_current_process_page_permission(ulong_t uvaddr, size_t size, bool* out_allowed_read_write);

#endif // __ASSEMBLY__
#endif // SECAGENT_WIN_MEMORY_H