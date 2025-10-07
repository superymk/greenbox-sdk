// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

void perf_counter_inc(perf_counter_t* cnt)
{
	*cnt = *cnt + 1;
}