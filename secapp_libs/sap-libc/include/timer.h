// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _SECPROCESS_TIMER_H
#define _SECPROCESS_TIMER_H

/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC    1000L
#define USEC_PER_MSEC   1000L
#define NSEC_PER_USEC   1000L
#define NSEC_PER_MSEC   1000000L
#define USEC_PER_SEC    1000000L
#define NSEC_PER_SEC    1000000000L

#ifndef __ASSEMBLY__

/// @brief Return (t2 - t1) in micro-seconds.
/// Requirement: t2 is later than t1.
/// @param t1 
/// @param t2 
/// @return 
extern uint64_t timer_elapsed_us(struct timeval* t1, struct timeval* t2);

/// @brief Return (t2 - t1) in milli-seconds.
/// Requirement: t2 is later than t1.
/// @param t1 
/// @param t2 
/// @return 
extern uint64_t timer_elapsed_ms(struct timeval* t1, struct timeval* t2);

extern SAP_STATUS timer_diff(sos_timespec_t* t1, sos_timespec_t* t2, sos_timespec_t* out_diff);

extern SAP_STATUS timer_diff_timeval(struct timeval* t1, struct timeval* t2, struct timeval* out_diff);

#endif // __ASSEMBLY__
#endif // _SECPROCESS_TIMER_H

