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

/* note these headers are all provided by newlib - you don't need to provide them */
#include <errno.h>
#include <reent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/ipc/comm.h>
#include <sys/ipc/varbuf.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_cmds.h"
#include "../gloss/_libc_ops.h"

// Parameters used to convert the timespec values:
#define MSEC_PER_SEC 1000L
#define USEC_PER_MSEC 1000L
#define NSEC_PER_USEC 1000L
#define NSEC_PER_MSEC 1000000L
#define USEC_PER_SEC 1000000L
#define NSEC_PER_SEC 1000000000L

#define is_timer_empty(timer) (unlikely(timer.val == 0))

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

// Empty timer. this value is readonly.
static sos_timespec_t empty_timer = {.val = 0};

/// @brief If a >= b returns true, otherwise false.
/// @param a
/// @param b
/// @return
static bool _timer_compare(sos_timespec_t a, sos_timespec_t b)
{
    if (a.tv_sec > b.tv_sec)
        return true;
    else if (a.tv_sec == b.tv_sec)
    {
        if (a.tv_nsec >= b.tv_nsec)
            return true;
    }

    return false;
}

static volatile uint64_t rdtscll(void)
{
    uint32_t low, high;

    mfence();
    // [NOTE] Do not use cpuid, otherwise it will trap to hypervisor unconditionally
    asm volatile("rdtsc" : "=a"(low), "=d"(high)::);

    return UINT32_TO_64(high, low);
}

// Sample the current timer value.
static volatile uint64_t timer_read(void)
{
    return rdtscll();
}

/// @brief Check if the current time is past a previously calculated end time.
static bool timer_check(sos_timespec_t end)
{
    sos_timespec_t now;
    VKSTATUS sos_status;

    sos_status = sos_sys_time_get_global(&now);
    if (FAIL(sos_status))
    {
        printf("<timer_check> Get current kernel time error!\n");
        return false;
    }

    return _timer_compare(now, end);
}

// Return the TSC value that is 'usecs' time in the future.
static uint64_t timer_calc_usec(unsigned long usecs, unsigned long tsc_per_us)
{
    return (uint64_t)(timer_read() + (uint64_t)tsc_per_us * (uint64_t)usecs);
}

// Private implementation of adding time.
static void _timer_add_us(sos_timespec_t *time, uint32_t usecs)
{
    uint64_t old_value = ((uint64_t)time->tv_sec) * NSEC_PER_SEC + time->tv_nsec;
    uint64_t new_value = old_value + ((uint64_t)usecs) * NSEC_PER_USEC;

    time->tv_sec = new_value / NSEC_PER_SEC;
    time->tv_nsec = new_value % NSEC_PER_SEC;
}

static sos_timespec_t timer_add_usec(uint32_t usecs)
{
    sos_timespec_t now;
    VKSTATUS status;

    status = sos_sys_time_get_global(&now);
    if (FAIL(status))
    {
        printf("<timer_add_usec> Timer read error\n");
        return empty_timer;
    }

    _timer_add_us(&now, usecs);
    return now;
}

int udelay_tsc(unsigned long usecs)
{
    VKSTATUS status = SUCCESS;
    uint64_t cpu_freq = 0;
    volatile uint64_t end_time = 0;

    status = sos_sys_get_current_cpu_boot_freq(&cpu_freq);
    if (FAIL(status))
        return -2;
    end_time = timer_calc_usec(usecs, cpu_freq / (1000 * 1000));
    mfence();

    while (timer_read() < end_time)
    {
        asm("pause");
    } // busy spin-lock

    return 0;
}

/// @brief Sleep for <__useconds> micro-seconds. Yield CPU to another SecProcess if sleeping for >= 5ms.
/// @param __useconds
/// @return Return -2 if failed to get CPU frequency.
int usleep(useconds_t __useconds)
{
    if (__useconds < 5 * USEC_PER_MSEC)
    {
        // If usleep for < 1ms, then we treat it as udelay; i.e., no CPU yield to another SecProcess.
        return udelay_tsc(__useconds);
    }
    else
    {
        // If sleep longer, then yield CPU.
        sos_timespec_t end = timer_add_usec(__useconds);

        if (is_timer_empty(end))
        {
            return 1;
        }

        while (1)
        {
            if (timer_check(end))
            {
                break;
            }
            sp_yield();
        }

        return 0;
    }
}

// [TODO] [Ticket 148] The implementation of sleep is not portable
unsigned int sleep(unsigned int seconds)
{
    return usleep(seconds * USEC_PER_SEC);
}

// Choose a conservative chunk to avoid very long single busy-waits.
// 10,000,000 µs = 10s per chunk. Adjust if you prefer shorter/longer spins.
#ifndef UDELAY_CHUNK_USEC
#define UDELAY_CHUNK_USEC 10000000UL
#endif

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    // Validate arguments per POSIX.
    if (!req || req->tv_nsec < 0 || req->tv_nsec >= 1000000000L || req->tv_sec < 0) {
        errno = EINVAL;
        return -1;
    }

    // Total microseconds, rounding UP so we don't sleep less than requested.
    // Use 128-bit math to avoid overflow when tv_sec is large.
    unsigned __int128 total_us = (unsigned __int128)req->tv_sec * 1000000u
                               + (unsigned __int128)((req->tv_nsec + 999) / 1000);

    if (total_us == 0) {
        if (rem) { rem->tv_sec = 0; rem->tv_nsec = 0; }
        return 0;
    }

    // Busy-wait in chunks using usleep()
    while (total_us > 0) {
        unsigned long this_chunk;

        // Cap each chunk to something reasonable
        this_chunk = min((unsigned long)total_us, (unsigned long)UDELAY_CHUNK_USEC);

        int rc = usleep(this_chunk);
        if (rc != 0) {
            // usleep() returns -2 when it can't get CPU freq (per your code).
            // Map to a sensible errno.
            if (rem) {
                unsigned __int128 remaining = total_us; // we didn't complete this_chunk
                rem->tv_sec  = (time_t)(remaining / 1000000u);
                rem->tv_nsec = (long)((remaining % 1000000u) * 1000u);
            }
            errno = (rc == -2) ? ENOSYS : EIO;
            return -1;
        }

        total_us -= this_chunk;
    }

    if (rem) { rem->tv_sec = 0; rem->tv_nsec = 0; }
    return 0;
}