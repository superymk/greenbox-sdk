// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_LINUX_MEMORY_H
#define SECAGENT_LINUX_MEMORY_H

#include <sa_headers.h>

/// @brief Allocate zeroed memory
/// @param size Size in bytes
/// @return Pointer or NULL on failure
extern void* linux_mm_malloc_zeroed(size_t size);

/// @brief Free memory
/// @param ptr Pointer
extern void linux_mm_free(void* ptr);

/// @brief Convert virtual address to physical address
/// @param vaddr Virtual address
/// @return Physical address
extern paddr_t linux_mm_vaddr_to_paddr(vaddr_t vaddr);

/// @brief Allocate pages
/// @param num_pages Number of pages
/// @param type Allocation type
/// @return Virtual address or INVALID_VADDR
extern vaddr_t linux_mm_alloc_pages(uint32_t num_pages, uint32_t type);

/// @brief Allocate physically contiguous pages
/// @param num_pages Number of pages
/// @param type Allocation type
/// @return Virtual address or INVALID_VADDR
extern vaddr_t linux_mm_alloc_physical_contiguous_pages(uint32_t num_pages, uint32_t type);

/// @brief Allocate physically contiguous pages (nocache)
/// @param num_pages Number of pages
/// @param type Allocation type
/// @return Virtual address or INVALID_VADDR
extern vaddr_t linux_mm_alloc_physical_contiguous_pages_nocache(uint32_t num_pages, uint32_t type);

/// @brief Free pages
/// @param vaddr Virtual address
/// @param num_pages Number of pages
/// @param type Allocation type
extern void linux_mm_free_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);

/// @brief Free physically contiguous pages
/// @param vaddr Virtual address
/// @param num_pages Number of pages
/// @param type Allocation type
extern void linux_mm_free_physical_contiguous_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);

/// @brief Map physical memory (nocache)
/// @param paddr Physical address
/// @param size Size in bytes
/// @return Mapped pointer or NULL
extern void* linux_ioremap_nocache(paddr_t paddr, size_t size);

/// @brief Unmap IO memory
/// @param ptr Mapped pointer
/// @param size Size in bytes
extern void linux_iounmap(void* ptr, size_t size);

/// @brief Set memory
/// @param ptr Destination
/// @param v Value
/// @param size Size in bytes
extern void linux_memset(void* ptr, char v, size_t size);

/// @brief Copy memory
/// @param __to Destination
/// @param __from Source
/// @param __n Size in bytes
extern void linux_memcpy(void *__to, const void *__from, size_t __n);

/// @brief Copy to user
/// @param usr_dest User destination
/// @param kern_src Kernel source
/// @param size Size in bytes
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS linux_copy_to_user(void* usr_dest, const void* kern_src, size_t size);

/// @brief Copy from user
/// @param kern_dest Kernel destination
/// @param usr_src User source
/// @param size Size in bytes
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS linux_copy_from_user(void* kern_dest, const void* usr_src, size_t size);

/// @brief Flush cache lines over range
/// @param vaddr Virtual address
/// @param size Size in bytes
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS linux_cache_flush_range(vaddr_t vaddr, size_t size);

/// @brief Memory fence
extern void linux_mm_fence(void);

/// @brief Check current process page permissions
/// @param uvaddr User virtual address
/// @param size Size in bytes
/// @param out_allowed_read_write Output permission flag
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS linux_mm_check_current_process_page_permission(ulong_t uvaddr, size_t size, bool* out_allowed_read_write);

#endif // SECAGENT_LINUX_MEMORY_H