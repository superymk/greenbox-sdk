// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>


bool ulong_test_highest_bit(ulong_t v, uint32_t *ans)
{
    uint32_t r = 0;

    if (v == 0) {
        return false;
    }

    while (v >>= 1) {
        r++;
    }

    *ans = r;
    return true;
}
