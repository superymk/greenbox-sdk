// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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