// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAGENT_PERF_H
#define SECAGENT_PERF_H
#include <os_types.h>


#ifndef __ASSEMBLY__
typedef uint64_t perf_counter_t;

/// @brief Increment performance counter
/// @param cnt Counter pointer
extern void perf_counter_inc(perf_counter_t* cnt);


#endif // __ASSEMBLY__
#endif // SECAGENT_PERF_H