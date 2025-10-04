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

#ifndef SECAGENT_PERF_H
#define SECAGENT_PERF_H
#include <os_types.h>


#ifndef __ASSEMBLY__
typedef uint64_t perf_counter_t;

extern void perf_counter_inc(perf_counter_t* cnt);


#endif // __ASSEMBLY__
#endif // SECAGENT_PERF_H