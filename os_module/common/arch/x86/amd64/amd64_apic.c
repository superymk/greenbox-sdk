/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#include <sa_headers.h>

#ifdef __amd64__

//! Get the physical address of the APIC registers page
//! make sure you map it to virtual memory
paddr_t lapic_get_base(void)
{
	uint32_t eax = 0, edx = 0;
	uint64_t msr = 0;

	msr = os->read_msr(IA32_APIC_BASE_MSR);
	eax = msr & 0xFFFFFFFF;
	edx = msr >> 32;

	return (eax & PAGE_MASK_4K) | (((ulong_t)(edx & 0x0fUL)) << 32ULL);
}

#endif // __amd64__