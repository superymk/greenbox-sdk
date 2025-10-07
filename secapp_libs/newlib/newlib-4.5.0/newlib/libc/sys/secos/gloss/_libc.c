// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sys/secapp_libc.h>
#include <stddef.h>
#include <sys/ipc/comm.h>
#include <stdio.h>
#include <stdlib.h>

#include "_libc_ops.h"
#include "_dbg_print.h"

static uint32_t sp_sapid;
static void* sp_heap_base = NULL;
static size_t sp_heap_size;
static void* sp_sharedmem_base = NULL;
static size_t sp_sharedmem_size;

uint32_t   end; // this global variable marks the end of the heap, required by libnosys/sbrk.c
void libc_init(uint32_t sp_id, void* heap_base, size_t heap_size, void* ipc_app_base, 
	size_t ipc_app_size, void* sharedmem_base, size_t sharedmem_size)
{
	sp_sapid = sp_id;
	sp_heap_base = heap_base;
	sp_heap_size = heap_size;
	sp_sharedmem_base = sharedmem_base;
	sp_sharedmem_size = sharedmem_size;

	// [NOTE] -4 is because the SecApp cannot access the address at 
	// (heap_base + heap_size)
	// [TODO] Double check if we set <end> correctly
	end = *(uint32_t*)((unsigned long)heap_base + heap_size - sizeof(end));

	// Disable buffer for printf
	setbuf(stdin, NULL);
  	setbuf(stdout, NULL);

	// Init IPC communication
	sp_comm_app_init(ipc_app_base);
}

//! Finalize Libc
void libc_fini(void)
{
	// Finalize IPC communication
	sp_comm_app_fini();
}

inline void* sp_get_heap_base(void)
{
	return sp_heap_base;
}

//! The empty space after the heap to avoid out of boundary access
#define HEAP_SAFEGUARD_GAP	32

inline size_t sp_get_heap_size(void)
{
	return sp_heap_size - HEAP_SAFEGUARD_GAP;
}

inline void* sp_get_sharedmem_base(void)
{
	return sp_sharedmem_base;
}

inline size_t sp_get_sharedmem_size(void)
{
	return sp_sharedmem_size;
}


inline uint32_t sp_get_sapid(void)
{
	return sp_sapid;
}