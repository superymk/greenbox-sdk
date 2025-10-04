#ifndef SECAGENT_LINUX_MEMORY_H
#define SECAGENT_LINUX_MEMORY_H

#include <sa_headers.h>

extern void* linux_mm_malloc_zeroed(size_t size);

extern void linux_mm_free(void* ptr);

extern paddr_t linux_mm_vaddr_to_paddr(vaddr_t vaddr);

extern vaddr_t linux_mm_alloc_pages(uint32_t num_pages, uint32_t type);
extern vaddr_t linux_mm_alloc_physical_contiguous_pages(uint32_t num_pages, uint32_t type);
extern vaddr_t linux_mm_alloc_physical_contiguous_pages_nocache(uint32_t num_pages, uint32_t type);

extern void linux_mm_free_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);
extern void linux_mm_free_physical_contiguous_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type);

extern void* linux_ioremap_nocache(paddr_t paddr, size_t size);

extern void linux_iounmap(void* ptr, size_t size);


extern void linux_memset(void* ptr, char v, size_t size);
extern void linux_memcpy(void *__to, const void *__from, size_t __n);
extern OSSTATUS linux_copy_to_user(void* usr_dest, const void* kern_src, size_t size);
extern OSSTATUS linux_copy_from_user(void* kern_dest, const void* usr_src, size_t size);

extern OSSTATUS linux_cache_flush_range(vaddr_t vaddr, size_t size);

extern void linux_mm_fence(void);

extern OSSTATUS linux_mm_check_current_process_page_permission(ulong_t uvaddr, size_t size, bool* out_allowed_read_write);

#endif // SECAGENT_LINUX_MEMORY_H