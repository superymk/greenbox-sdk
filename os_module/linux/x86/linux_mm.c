// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <linux/slab.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/string.h> 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#include "linux_mm.h"


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
	//#include <asm-generic/io.h> // ioremap_nocache
#else
	#include <asm/io.h>
#endif

//! Return NULL on error
void* linux_mm_malloc_zeroed(size_t size)
{
	void* ptr = NULL;

	// Check argument
	if (!size)
		return NULL;

	// Allocate memory, and zero the memory if allocated
	ptr = kmalloc(size, GFP_KERNEL);
	if(ptr)
		memset(ptr, 0, size);

	return ptr;
}

void linux_mm_free(void* ptr)
{
	kfree(ptr);
}

// Return the upper value of integer N, where N = log2(value)
static uint32_t upper_order_base2(uint32_t value)
{
	uint32_t result = 0;

	while(value != 0)
	{
		value = value >> 1;
		result++;
	}

	return result;
}

vaddr_t linux_mm_alloc_pages(uint32_t num_pages, uint32_t type)
{
	vaddr_t vaddr = INVALID_VADDR;

	if (type == ALLOC_ORDINARY_MEMORY)
	{
		uint32_t order = upper_order_base2(num_pages);

		vaddr = (vaddr_t)(__get_free_pages(GFP_KERNEL, order));
		if(!vaddr)
		{
			DRV_ERR("Allocate pages error. num_pages:%u\n", num_pages);
			return INVALID_VADDR;
		}
	}
	else if (type == ALLOC_DMA_MEMORY)
	{
		return linux_mm_alloc_physical_contiguous_pages_nocache(num_pages, type);
	}

	return vaddr;
}

vaddr_t linux_mm_alloc_physical_contiguous_pages(uint32_t num_pages, uint32_t type)
{
	uint32_t order = upper_order_base2(num_pages);
	vaddr_t vaddr = INVALID_VADDR;
	
	if (type == ALLOC_ORDINARY_MEMORY)
		vaddr = (vaddr_t)(__get_free_pages(GFP_KERNEL, order));
	else if (type == ALLOC_DMA_MEMORY)
		vaddr = (vaddr_t)(__get_free_pages(GFP_DMA, order));

	if(!vaddr)
	{
		DRV_ERR("Allocate physical contiguous pages error. num_pages:%u, type:%u\n", num_pages, type);
		return INVALID_VADDR;
	}

	return vaddr;
}

vaddr_t linux_mm_alloc_physical_contiguous_pages_nocache(uint32_t num_pages, uint32_t type)
{
	uint32_t order = upper_order_base2(num_pages);
	vaddr_t vaddr = INVALID_VADDR;
	
	if (type == ALLOC_ORDINARY_MEMORY)
		vaddr = (vaddr_t)(__get_free_pages(GFP_KERNEL, order));
	else if (type == ALLOC_DMA_MEMORY)
		vaddr = (vaddr_t)(__get_free_pages(GFP_DMA, order));

	if(!vaddr)
	{
		DRV_ERR("Allocate physical contiguous pages error. num_pages:%u, type:%u\n", num_pages, type);
		return INVALID_VADDR;
	}

	return vaddr;
}

void linux_mm_free_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type)
{
	uint32_t order = upper_order_base2(num_pages);

	(void)type;
	free_pages((unsigned long)vaddr, order);
}

void linux_mm_free_physical_contiguous_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type)
{
	linux_mm_free_pages(vaddr, num_pages, type);
}

paddr_t linux_mm_vaddr_to_paddr(vaddr_t vaddr)
{
	return (paddr_t)virt_to_phys((void*)vaddr);
}

void* linux_ioremap_nocache(paddr_t paddr, size_t size)
{
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
        // From Linux 5.6: The default ioremap() behavior is non-cached
        return ioremap(paddr, size);
    #else
        return ioremap_nocache(paddr, size);
    #endif
}

void linux_iounmap(void* ptr, size_t size)
{
	iounmap(ptr);
}

void linux_memset(void* ptr, char v, size_t size)
{
	memset(ptr, v, size);
}

void linux_memcpy(void *__to, const void *__from, size_t __n)
{
	memcpy(__to, __from, __n);
}

OSSTATUS linux_copy_to_user(void* usr_dest, const void* kern_src, size_t size)
{
	int result = copy_to_user(usr_dest, kern_src, size);

	if(result != 0)
		return E_INTERNAL_ERROR;

	return OS_SUCCESS;
}

OSSTATUS linux_copy_from_user(void* kern_dest, const void* usr_src, size_t size)
{
	int result = copy_from_user(kern_dest, usr_src, size);

	if(result != 0)
		return E_INTERNAL_ERROR;

	return OS_SUCCESS;
}


extern bool cpu_support_clflush;

static inline void clflush_ins(volatile void *p)
{
	asm volatile ("clflush (%0)" :: "r"(p));
}

OSSTATUS linux_cache_flush_range(vaddr_t vaddr, size_t size)
{
	size_t clflush_size;
	vaddr_t p, vend;

	// Check: The current CPU must support CLFLUSH
	if(!cpu_support_clflush)
		return E_INV_ARG;

	clflush_size = ((cpuid_ebx(1) >> 8) & 0xff) << 3;
	p = (vaddr_t)vaddr & ~(clflush_size - 1);
	vend = vaddr + size;

	// Check: No overflow of (vaddr + size)
	// [NOTE] <p> also tells us the start of the aligned cache line 
	if (p >= vend)
		return E_INV_ARG;

	for (; p < vend; p += clflush_size)
		clflush_ins((void*)p);

	return OS_SUCCESS;
}

void linux_mm_fence(void)
{
	__asm__ __volatile__("mfence": : :"memory");
}

OSSTATUS linux_mm_check_current_process_page_permission(ulong_t uvaddr, size_t size, bool* out_allowed_read_write)
{
	ulong_t offset = 0;
	struct task_struct *task = current;   // Current process's task_struct
    struct mm_struct *mm = task->mm;
    struct vm_area_struct *vma;

	// Check: parameters must be valid
	if(!uvaddr || !size || !out_allowed_read_write)
		return E_INV_ARG;

	for(offset = 0; offset < size; offset += PAGE_SIZE)
	{
		ulong_t cur_uvaddr_aligned = (uvaddr + offset) & PAGE_MASK;

		// Find the VM area that contains the virtual address
		vma = find_vma(mm, cur_uvaddr_aligned);
		if (vma && cur_uvaddr_aligned >= vma->vm_start && cur_uvaddr_aligned < vma->vm_end) 
		{
			// Check read and write permissions from the VMA
			if (!(vma->vm_flags & (VM_READ | VM_WRITE)))
			{
				// If found a page that the current process does not have RW permission, then set <out_allowed_read_write> 
				// to be false and return.
				*out_allowed_read_write = false;
				return OS_SUCCESS;
			}
		}
	}

	// The current process has RW permission to all the memory [uvaddr, uvaddr + size)
	*out_allowed_read_write = true;
	return OS_SUCCESS;
}