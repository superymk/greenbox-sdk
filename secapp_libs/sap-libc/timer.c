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

#include <palddk.h>
#include <stdio.h>

uint64_t timer_elapsed_us(struct timeval* t1, struct timeval* t2)
{
	uint64_t t2_usec = ((uint64_t)t2->tv_sec * (uint64_t)USEC_PER_SEC) + ((uint64_t)t2->tv_usec);
	uint64_t t1_usec = ((uint64_t)t1->tv_sec * (uint64_t)USEC_PER_SEC) + ((uint64_t)t1->tv_usec);

	return t2_usec - t1_usec;
}

uint64_t timer_elapsed_ms(struct timeval* t1, struct timeval* t2)
{
	uint64_t t2_msec = ((uint64_t)t2->tv_sec * (uint64_t)MSEC_PER_SEC) + ((uint64_t)t2->tv_usec / USEC_PER_MSEC);
	uint64_t t1_msec = ((uint64_t)t1->tv_sec * (uint64_t)MSEC_PER_SEC) + ((uint64_t)t1->tv_usec / USEC_PER_MSEC);

	return t2_msec - t1_msec;
}

SAP_STATUS timer_diff(sos_timespec_t* t1, sos_timespec_t* t2, sos_timespec_t* out_diff)
{
    uint64_t v1 = 0;
    uint64_t v2 = 0;

    if(!t1 || !t2 || !out_diff)
        return EINVAL;
    
    v1 = ((uint64_t)t1->tv_sec * (uint64_t)NSEC_PER_SEC) + (uint64_t)t1->tv_nsec;
    v2 = ((uint64_t)t2->tv_sec * (uint64_t)NSEC_PER_SEC) + (uint64_t)t2->tv_nsec;

    out_diff->tv_sec  = (v2 - v1) / NSEC_PER_SEC;
    out_diff->tv_nsec  = (v2 - v1) % NSEC_PER_SEC;

    // On success
    return SUCCESS;
}

SAP_STATUS timer_diff_timeval(struct timeval* t1, struct timeval* t2, struct timeval* out_diff)
{
    uint64_t v1 = 0;
    uint64_t v2 = 0;

    if(!t1 || !t2 || !out_diff)
        return EINVAL;
    
    v1 = ((uint64_t)t1->tv_sec * (uint64_t)USEC_PER_SEC) + (uint64_t)t1->tv_usec;
    v2 = ((uint64_t)t2->tv_sec * (uint64_t)USEC_PER_SEC) + (uint64_t)t2->tv_usec;

    out_diff->tv_sec  = (v2 - v1) / USEC_PER_SEC;
    out_diff->tv_usec  = (v2 - v1) % USEC_PER_SEC;

    // On success
    return SUCCESS;
}