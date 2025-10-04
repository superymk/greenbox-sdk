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

#ifndef SECAPP_LIBC_EXTENSION_STL_MATH_H
#define SECAPP_LIBC_EXTENSION_STL_MATH_H

#ifndef __ASSEMBLY__

#define ABS(x)	((x) < 0 ? -(x) : (x))

#define ROUND_UP_TO_NEXT_POWER_OF_2(x) \
    (--(x), \
    (x) |= (x) >> 1, \
    (x) |= (x) >> 2, \
    (x) |= (x) >> 4, \
    (x) |= (x) >> 8, \
    (x) |= (x) >> 16, \
    (x) |= ((unsigned long long)x) >> 32ULL, \
    ++(x))

extern uint32_t ROUND_UP_TO_NEXT_POWER_OF_2_uint32(uint32_t x);



#endif // __ASSEMBLY__
#endif // SECAPP_LIBC_EXTENSION_STL_MATH_H