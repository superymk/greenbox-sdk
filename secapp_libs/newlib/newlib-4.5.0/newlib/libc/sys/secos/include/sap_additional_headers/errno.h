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

#ifndef SECAPP_ERRNO_H
#define SECAPP_ERRNO_H

#include <vk_errno.h>

#define EAGAIN      		    11  // Try again
#define E_SECOS_SYSCALL         1000


#ifndef __ASSEMBLY__

typedef ulong_t 				SAP_STATUS;

#endif // __ASSEMBLY__
#endif // SECAPP_ERRNO_H