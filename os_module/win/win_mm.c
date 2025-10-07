// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)
// #include <ntddk.h> // If <ntifs.h> is used, then using <ntddk.h> causes conflicts of type definitions.
#include <ntifs.h>

#include "win_mm.h"

void* win_mm_malloc_zeroed(size_t size)
{
	PVOID ptr = NULL;

	// Check argument
	if (!size)
		return NULL;

	// Allocate memory, and zero the memory if allocated
	ptr = (PVOID)ExAllocatePool2 (POOL_FLAG_NON_PAGED, size, SA_TAG);
	
	return (void*)ptr;
}

void win_mm_free(void* ptr)
{
	ExFreePool ((PVOID)ptr);
}

static int win_touch_mem(void *ptr, size_t len)
{
	int i;

	for(i=0; i < len; i += PAGE_SIZE_4K) 
	{
		volatile unsigned int *addr = (unsigned int*)((vaddr_t)ptr + i);
		volatile unsigned int data = *addr;

		// Write non-zero values to each page to prevent OS mapping empty pages to the same physical memory page.
		*addr = (i+1);
		os->mm_fence();
		*addr = data;
		os->mm_fence();

		(void) data; (void)addr;
	}
	return 0;
}

vaddr_t win_mm_alloc_pages(uint32_t num_pages, uint32_t type)
{
	vaddr_t result = INVALID_VADDR;

	if (type == ALLOC_ORDINARY_MEMORY)
	{
		// Why the return value is page aligned? See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepool2
		// "If NumberOfBytes is PAGE_SIZE or greater, a page-aligned buffer is allocated. Memory allocations of PAGE_SIZE or 
		// less are allocated within a page and do not cross page boundaries. Memory allocations of less than PAGE_SIZE are 
		// not necessarily page-aligned but are aligned to 8-byte boundaries in 32-bit systems and to 16-byte boundaries in 
		// 64-bit systems."
		result = (vaddr_t)ExAllocatePool2(POOL_FLAG_NON_PAGED, num_pages * PAGE_SIZE, SA_TAG);
	}
	else if (type == ALLOC_DMA_MEMORY)
	{
		result = (vaddr_t)MmAllocateNonCachedMemory(num_pages * PAGE_SIZE);
	}

	if(!result)
		return INVALID_VADDR;

	memset((void*)result, 0, num_pages * PAGE_SIZE);
	os->mm_fence();

	// Touch!
	win_touch_mem((void*)result, num_pages * PAGE_SIZE);

	return result;
}

vaddr_t win_mm_get_physical_contiguous_pages(uint32_t num_pages, uint32_t type)
{
	vaddr_t vaddr = INVALID_VADDR;
	PHYSICAL_ADDRESS l1, l2, l3;
	
	l1.QuadPart = 0;
	l2.QuadPart = -1;
	l3.QuadPart = 0;
	if (type == ALLOC_ORDINARY_MEMORY)
		vaddr = (vaddr_t)MmAllocateContiguousMemorySpecifyCache(num_pages * PAGE_SIZE, 
      		l1, l2, l3, MmCached);
	else if (type == ALLOC_DMA_MEMORY)
		vaddr = (vaddr_t)MmAllocateContiguousMemorySpecifyCache(num_pages * PAGE_SIZE, 
      		l1, l2, l3, MmNonCached);

	if(!vaddr)
		return INVALID_VADDR;

	memset((void*)vaddr, 0, num_pages * PAGE_SIZE);
	os->mm_fence();

	// Touch!
	win_touch_mem((void*)vaddr, num_pages * PAGE_SIZE);

	return vaddr;
}

vaddr_t win_mm_alloc_physical_contiguous_pages_nocache(uint32_t num_pages, uint32_t type)
{
	UNREFERENCED_PARAMETER(type);

	return win_mm_get_physical_contiguous_pages(num_pages, ALLOC_DMA_MEMORY);
}

void win_mm_free_pages(vaddr_t vaddr, uint32_t num_pages, uint32_t type)
{
	if (type == ALLOC_ORDINARY_MEMORY)
		ExFreePool ((PVOID)vaddr);
	else if (type == ALLOC_DMA_MEMORY)
		MmFreeNonCachedMemory((PVOID)vaddr, num_pages * PAGE_SIZE);
}

void win_mm_free_physical_contiguous_pages(vaddr_t vaddr, uint32_t num_pages, 
	uint32_t type)
{
	if (type == ALLOC_ORDINARY_MEMORY)
		MmFreeContiguousMemorySpecifyCache ((PVOID)vaddr, num_pages * PAGE_SIZE, MmCached);
	else if (type == ALLOC_DMA_MEMORY)
		MmFreeContiguousMemorySpecifyCache ((PVOID)vaddr, num_pages * PAGE_SIZE, MmNonCached);
}

paddr_t win_mm_vaddr_to_paddr(vaddr_t vaddr)
{
	paddr_t paddr = INVALID_PADDR;
	
#if defined (__i386__)
    paddr = MmGetPhysicalAddress ((PVOID)vaddr).LowPart;
#elif defined (__amd64__)
	paddr = MmGetPhysicalAddress ((PVOID)vaddr).QuadPart;
#endif

	return paddr;
}

void* win_ioremap_nocache(paddr_t paddr, size_t size)
{
	PVOID ptr = NULL;
	PHYSICAL_ADDRESS in_paddr;

	in_paddr.QuadPart = 0;
#if defined (__i386__)
    in_paddr.LowPart = paddr;
#elif defined (__amd64__)
	in_paddr.QuadPart = paddr;
#endif	

	ptr = MmMapIoSpace(in_paddr, size, MmNonCached);
	return (void*)ptr;
}

void win_iounmap(void* ptr, size_t size)
{
	MmUnmapIoSpace((PVOID)ptr, size);
}

void win_memset(void* ptr, char v, size_t size)
{
	memset(ptr, v, size);
}

void win_memcpy(void *__to, const void *__from, size_t __n)
{
	memcpy(__to, __from, __n);
}

OSSTATUS win_copy_to_user(void* usr_dest, const void* kern_src, size_t size)
{
	if(!usr_dest || !kern_src)
		return E_INV_ARG;
	
    __try
    {
        RtlCopyMemory(usr_dest, kern_src, size);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
		return E_INTERNAL_ERROR;
    }

	return OS_SUCCESS;
}

OSSTATUS win_copy_from_user(void* kern_dest, const void* usr_src, size_t size)
{
	if(!kern_dest || !usr_src)
		return E_INV_ARG;

	RtlZeroMemory((PVOID)kern_dest, size);
	__try
    {
        RtlCopyMemory(kern_dest, usr_src, size);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
		return E_INTERNAL_ERROR;
    }

	return OS_SUCCESS;
}

void win_mm_fence(void)
{
	MemoryBarrier();
}

void win_wbinvd(void)
{
	__wbinvd();
}

static uint32_t cpuid_ebx(uint32_t fn)
{
	int cpuInfo[4] = { 0 };
	
	__cpuid(cpuInfo, fn);
	return (uint32_t)cpuInfo[1];
}

extern bool cpu_support_clflush;

OSSTATUS win_cache_flush_range(vaddr_t vaddr, size_t size)
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
		_mm_clflush((void*)p);

	return OS_SUCCESS;
}

OSSTATUS win_mm_check_current_process_page_permission(ulong_t uvaddr, size_t size, bool* out_allowed_read_write)
{
	NTSTATUS status = STATUS_SUCCESS;
    // PEPROCESS currentProcess = PsGetCurrentProcess();
    MEMORY_BASIC_INFORMATION memInfo;
	ulong_t offset = 0;

	// Check: parameters must be valid
	if(!uvaddr || !size || !out_allowed_read_write)
		return E_INV_ARG;

	for(offset = 0; offset < size; offset += PAGE_SIZE)
	{
		ulong_t cur_uvaddr_aligned = (uvaddr + offset) & PAGE_MASK_4K;

		// Query memory information
		status = ZwQueryVirtualMemory(
			NtCurrentProcess(),
			(PVOID)cur_uvaddr_aligned,
			MemoryBasicInformation,
			&memInfo,
			sizeof(memInfo),
			NULL
		);

		if (!NT_SUCCESS(status))
			return E_INTERNAL_ERROR;

		if (!(memInfo.Protect & PAGE_READWRITE))
		{
			// If found a page that the current process does not have RW permission, then set <out_allowed_read_write> 
			// to be false and return.
			*out_allowed_read_write = false;
			return OS_SUCCESS;
		}
	}

	// The current process has RW permission to all the memory [uvaddr, uvaddr + size)
	*out_allowed_read_write = true;
	return OS_SUCCESS;
}