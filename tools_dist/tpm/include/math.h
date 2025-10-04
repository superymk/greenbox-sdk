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

#ifndef STL_MATH_H
#define STL_MATH_H

#ifndef __ASSEMBLY__

#ifndef max
#define max(x, y) ( ((x)>(y))?(x):(y) )
#endif // max

#ifndef min
#define min(x, y) ( ((x)<(y))?(x):(y) )
#endif // min

#endif // __ASSEMBLY__
#endif // STL_MATH_H