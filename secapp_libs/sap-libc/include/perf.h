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

#ifndef _SP_PERF_H
#define _SP_PERF_H



#ifndef __ASSEMBLY__

extern uint64_t sp_perf_rdtscll(void);




/********* One-Shot Perf Recording *********/
#define PERF_ONESHOT_BEGIN()	perf_oneshot_begin()
#define PERF_ONESHOT_END()		perf_oneshot_end(__FILE__, __func__, __LINE__)

/// @brief Measure the elapsed time (in CPU cycles) between <perf_oneshot_begin> and <perf_oneshot_end>, exclude the time
/// paused and resumed in between.
/// @param  
extern void perf_oneshot_begin(void);

/// @brief Measure the elapsed time (in CPU cycles) between <perf_oneshot_begin> and <perf_oneshot_end>, exclude the time
/// paused and resumed in between.
/// @param  
extern void perf_oneshot_end(const char* file_name, const char* func_name, const int line);

/// @brief Pause the timer between <perf_oneshot_begin> and <perf_oneshot_end>
/// Do nothing on re-entry without resuming.
/// @param  
extern void perf_oneshot_pause(void);

/// @brief Resume the timer between <perf_oneshot_begin> and <perf_oneshot_end>.
/// Do nothing on re-entry without pausing.
/// @param  
extern void perf_oneshot_resume(void);




/********* Performance Assertion *********/

/// @brief Begin measuring the clockcycles between <perf_assert_begin> and <perf_assert_end>.
/// @param  
extern void perf_assert_begin(void);

/// @brief End measuring the clockcycles. If it takes longer than <elapsed_tsc_limit>, then the function prints a warning.
/// @param tag 
/// @param elapsed_tsc_limit 
extern void perf_assert_end(char* tag, uint64_t elapsed_tsc_limit);

#endif // __ASSEMBLY__
#endif // _SP_PERF_H