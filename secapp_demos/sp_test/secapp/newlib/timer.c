// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <unistd.h>
#include <sys/time.h>

#include <sap_test.h>

bool test_timer_sleep_long(void)
{
    const uint32_t SLEEP_SECONDS = 30;
    sos_timespec_t t1 = {0}, t2 = {0}, t = {0};
    struct timeval tv1 = {0}, tv2 = {0}, tv = {0};
    struct timeval tv1_red_os = {0}, tv2_red_os = {0}, tv_red_os = {0};

    SAP_ERR("<test_timer_sleep_long> Sleep for %u seconds.\n", SLEEP_SECONDS);

    (void)gettimeofday_untrusted(&tv1_red_os, NULL);
    sos_sys_time_get_global(&t1);
    (void)gettimeofday(&tv1, NULL);
    
    sleep(SLEEP_SECONDS);
    // usleep(1000* 1000 * SLEEP_SECONDS);

    sos_sys_time_get_global(&t2);
    (void)gettimeofday(&tv2, NULL);
    (void)gettimeofday_untrusted(&tv2_red_os, NULL);

    (void)timer_diff(&t1, &t2, &t);
    (void)timer_diff_timeval(&tv1, &tv2, &tv);
    (void)timer_diff_timeval(&tv1_red_os, &tv2_red_os, &tv_red_os);

    SAP_ERR("<sos_sys_time_get_global> Diff: tv_sec:%u, tv_nsec:%u\n", t.tv_sec, t.tv_nsec);
    SAP_ERR("<gettimeofday> Diff: tv_sec:%lu, tv_usec:%lu\n", tv.tv_sec, tv.tv_usec);
    SAP_ERR("<gettimeofday_untrusted> Diff: tv_sec:%lu, tv_usec:%lu\n", tv_red_os.tv_sec, tv_red_os.tv_usec);

    SAP_ERR("\n");
    SAP_ERR("<gettimeofday> tv1_sec:%lu, tv1_usec:%lu\n", tv1.tv_sec, tv1.tv_usec);
    SAP_ERR("<gettimeofday_untrusted> tv1_sec:%lu, tv1_usec:%lu\n", tv1_red_os.tv_sec, tv1_red_os.tv_usec);
    SAP_ERR("<gettimeofday> tv2_sec:%lu, tv2_usec:%lu\n", tv2.tv_sec, tv2.tv_usec);
    SAP_ERR("<gettimeofday_untrusted> tv2_sec:%lu, tv2_usec:%lu\n", tv2_red_os.tv_sec, tv2_red_os.tv_usec);

    return true;
}