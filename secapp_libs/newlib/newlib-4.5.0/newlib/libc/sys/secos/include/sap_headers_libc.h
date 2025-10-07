// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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