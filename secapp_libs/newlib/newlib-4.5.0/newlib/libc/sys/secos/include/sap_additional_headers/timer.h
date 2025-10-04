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

#ifndef SAP_TIMER_H
#define SAP_TIMER_H


#ifndef __ASSEMBLY__


/// @brief Init wall clock timer in SecOS
/// @param  
/// @return Return 0 on success. Return -1 on errors.
extern int secos_init_wall_clock_timer(void);

/// @brief Call the <gettimeofday> function of untrusted OS.
/// @param p 
/// @param tz 
/// @return 
extern int gettimeofday_untrusted(struct timeval *p, void* tz);

/// @brief Delay for <usecs> micro-seconds without yielding the CPU.
/// @param usecs 
/// @return Return -2 if failed to get CPU frequency.
extern int udelay_tsc(unsigned long usecs);

#endif // __ASSEMBLY__
#endif // SAP_TIMER_H