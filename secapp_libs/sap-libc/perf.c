// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <palddk.h>
#include <stdio.h>

uint64_t sp_perf_rdtscll(void)
{
    uint32_t low, high;

	barrier();
	// [NOTE] Do not use cpuid, otherwise it will trap to hypervisor unconditionally
    asm volatile("rdtsc" : "=a" (low), "=d" (high)::);

    return UINT32_TO_64(high, low);
}




/********* One-Shot Perf Recording *********/
static bool _perf_oneshot_enabled = false;
static bool _perf_oneshot_paused = false;
static uint64_t _perf_oneshot_last_tsc = 0;
static uint64_t _perf_oneshot_delta_tsc = 0;

static inline void _perf_update_delta_tsc(void)
{
    uint64_t now = sp_perf_rdtscll();
    _perf_oneshot_delta_tsc += now - _perf_oneshot_last_tsc;
}

void perf_oneshot_begin(void)
{
    _perf_oneshot_delta_tsc = 0;
	_perf_oneshot_enabled = true;
	_perf_oneshot_last_tsc = sp_perf_rdtscll();
}

void perf_oneshot_pause(void)
{
    if(_perf_oneshot_enabled && !_perf_oneshot_paused)
    {
        _perf_update_delta_tsc();

        _perf_oneshot_paused = true;
    }
}

void perf_oneshot_resume(void)
{
    if(_perf_oneshot_enabled && _perf_oneshot_paused)
    {
        _perf_oneshot_last_tsc = sp_perf_rdtscll();

        _perf_oneshot_paused = false;
    }
}

void perf_oneshot_end(const char* file_name, const char* func_name, const int line)
{
	if(_perf_oneshot_enabled)
	{
		_perf_update_delta_tsc();
		printf("[SecProcess %u][%s:%s:%d] One-shot perf measure result:%llu cycles\n", 
            current_sp->sp_id,
            file_name, func_name, line, 
            (long long unsigned int)_perf_oneshot_delta_tsc
        );

		_perf_oneshot_enabled = false;
        _perf_oneshot_paused = false;
	}
}




/********* Performance Assertion *********/
static uint64_t perf_assert_begin_rdtsc = 0;

void perf_assert_begin(void)
{
    barrier();
    perf_assert_begin_rdtsc = sp_perf_rdtscll();
}

void perf_assert_end(char* tag, uint64_t elapsed_tsc_limit)
{
    uint64_t perf_assert_end_rdtsc = sp_perf_rdtscll();
    
    barrier();
    if(perf_assert_end_rdtsc - perf_assert_begin_rdtsc >= elapsed_tsc_limit)
    {
        // The time duration between <perf_assert_begin> and <perf_assert_end> is beyond our expectation. Print performance warning.
        printf("[Perf-Warning] %s. Elapsed time (clockcycles) %lu exceeds the limit (clockcycles) %lu\n", 
            tag, 
            (perf_assert_end_rdtsc - perf_assert_begin_rdtsc), elapsed_tsc_limit);
    }

    perf_assert_begin_rdtsc = 0;
    barrier();
}