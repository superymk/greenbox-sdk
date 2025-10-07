// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

void dbg_dumpmem(vaddr_t start_vaddr, size_t size)
{
	ulong_t i = 0, j = 0;

	DRV_INFO("Dumping memory 0x%lX, size:0x%lX", start_vaddr, (ulong_t)size);
	FOREACH_S(i, size, ADDR_MAX, 0, sizeof(ulong_t))
	{
		ulong_t* p = (ulong_t*)(start_vaddr + i);

		if(!(j%8))
			PRINT("\n\t\t");

		PRINT("0x%16lX ", *p);
		j++;
	}
	PRINT("\n");
}