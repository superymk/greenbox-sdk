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