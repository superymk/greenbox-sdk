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