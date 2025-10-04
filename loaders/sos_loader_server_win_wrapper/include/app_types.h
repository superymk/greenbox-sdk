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


#ifndef APP_TYPES_H
#define APP_TYPES_H

#ifndef __ASSEMBLY__

#if defined(__CYGWIN__) && defined (__amd64__)
typedef unsigned long uint64_t;
#else
typedef unsigned long long uint64_t;
#endif

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#endif // __ASSEMBLY__
#endif // APP_TYPES_H