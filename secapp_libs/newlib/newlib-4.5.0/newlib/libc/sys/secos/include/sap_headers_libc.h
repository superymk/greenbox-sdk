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

#ifndef SECAPP_HEADERS_LIBC_H
#define SECAPP_HEADERS_LIBC_H

// Include basic headers
#include <sap_additional_headers/types.h>
#include <sap_additional_headers/vars.h>
#include <sap_additional_headers/logs.h>
#include <sap_additional_headers/atomic.h>
#include <sap_additional_headers/errno.h>
#include <sap_additional_headers/lock.h>
#include <sap_additional_headers/tlsf.h>
#include <sap_additional_headers/mm.h>
#include <sap_additional_headers/sp.h>
#include <sap_additional_headers/libc_debug.h>
#include <sap_additional_headers/timer.h>

#include <sap_additional_headers/stl_math.h>
#include <sap_additional_headers/stl_dlist.h>

#include <sap_additional_headers/comm_sp_sp.h>

// Include SecOS's public headers
#include <secapp_defs.h>
#include <sos_common_defs.h>
#include <vk_errno.h>
#include <vk_mm_defs.h>
#include <vk_configs.h>
#include <vk_common_intfs.h>
#include <vk_syscalls.h>
#include <sos_sp_intfs.h>


#include <sap_additional_headers/syscalls.h>

#endif // SECAPP_HEADERS_LIBC_H