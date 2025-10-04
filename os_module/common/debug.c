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