// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file [NOTE] We do not name the file as "sp.c" because newlib-4.5.0 seems to have bugs to link all C files with the
/// same file name though they are in different sub-folders in newlib/libc/sys/secos. 

#include <stdlib.h>   // aligned_alloc, free
#include <stdint.h>   // SIZE_MAX
#include <errno.h>    // EINVAL, ENOMEM
#include <sap_headers_libc.h>

void sp_yield(void)
{
	sos_sys_sp_yield(current_sp->init_sp_id);
}