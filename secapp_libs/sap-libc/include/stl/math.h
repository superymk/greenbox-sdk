// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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