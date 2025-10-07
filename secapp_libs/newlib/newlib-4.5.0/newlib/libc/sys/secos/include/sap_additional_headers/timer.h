// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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