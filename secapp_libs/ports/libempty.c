// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <stdbool.h>

/// @file ./configure in some 3rd-party libraries need to compile dummy SecApps to detect compiler features. The process
/// needs this file to prevent compile errors.

typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;

typedef uint8_t 	intr_t; 

int __libc_start_main(int *(main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    return 0;
}

void sp_main(void* params)
{

}

void sp_on_exit(void* sp_params)
{

}

void sp_handle_intr(intr_t vector)
{

}

void sp_handle_sp_request(void* msg)
{

}

bool sp_handle_app_request(void* msg)
{
	return true;
}

void sp_handle_exception(void)
{

}

/********* sap-libc/pal_mm.c *********/
int pal_mm_create_pool(void* sp_params)
{
    return 0;
}

void pal_mm_destroy_pool(void)
{

}

/********* sap-libc/perf.c *********/
uint64_t sp_perf_rdtscll(void)
{
    return 0;
}
