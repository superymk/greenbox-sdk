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

#ifndef SECAPP_STL_MATH_H
#define SECAPP_STL_MATH_H

#ifndef __ASSEMBLY__

#define C_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_CLOSEST(x, divisor)({                 \
            typeof(divisor) __divisor = divisor;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN_DOWN(x,a)         ((x) & ~((typeof(x))(a)-1))

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define VALUE_IN_RANGE(v, x, y)                 ((v) >= (x) && (v) < (y))
#define VALUE_IN_RANGE_INCLUSIVE(v, x, y)       ((v) >= (x) && (v) <= (y))



#endif // __ASSEMBLY__
#endif // SECAPP_STL_MATH_H