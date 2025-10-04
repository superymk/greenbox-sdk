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

#ifndef _PALDEF_H
#define _PALDEF_H

// Include basic headers
// #include <sap_types.h>
// #include <sap_vars.h>
// #include <log.h>

#include <stddef.h>
#include <stdbool.h>
#include <endian.h>
#include <sys/cdefs.h>
#include <sap_headers_libc.h>
#include <stdio.h>



// #include <lock.h>
#include <sap_string.h>
#include <timer.h>
#include <perf.h>
#include <sp.h>
// #include <sap_errno.h>
// #include <stl/sap_math.h>
#include <x86/_endian.h>
#include <x86/atomic.h>
#include <x86/cpu.h>
#include <x86/mm.h>

#include <stl/math.h>
#include <stl/rb.h>

// Include SecOS's public headers
// #include <sos_common_defs.h>
// #include <vk_errno.h>
// #include <vk_mm_defs.h>
// #include <vk_configs.h>
// #include <vk_common_intfs.h>
// #include <vk_syscalls.h>
// #include <sos_sp_intfs.h>


//#include <pal_glibc.h>

//#include <sp_errno.h>
// #include <pal_types.h>
// #include <pal_configs.h>
// #include <pal_var.h>
// #include <locks.h>			// [TODO] Refactor it

// #include <app_sp_common_defs.h>

// #include <timer.h>

// #include <pal_log.h>
// #include <pal_x86.h>
// #include <pal_debug.h>


// #include <common_pci.h>
// #include <common_usb.h>
#include <sap_mm.h>
#include <common_mm.h>
// #include <comm/comm_isp.h>

#include <sp_debug.h>

// #include <syscalls.h>

// #include <pal_syscalls.h>

#endif // _PALDEF_H

