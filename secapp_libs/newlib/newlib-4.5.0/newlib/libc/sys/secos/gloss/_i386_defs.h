// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef I386_DEFINITIONS
#define I386_DEFINITIONS

// Include basic headers
#include <sap_additional_headers/types.h>

// Include SecOS's public headers
#include <sos_common_defs.h>
#include <vk_errno.h>
#include <vk_mm_defs.h>

#define barrier() __asm__ __volatile__("": : :"memory")

#endif // I386_DEFINITIONS