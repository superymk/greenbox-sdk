// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sys/types.h>

typedef unsigned __int128 u128;

/*
 * Software 128-bit unsigned division with remainder.
 * - Returns: quotient = dividend / divisor
 * - *rem (if non-NULL): remainder = dividend % divisor
 *
 * Notes:
 * - If divisor == 0, this routine returns all 1s (max u128) and sets
 *   *rem = dividend (matching common compiler-rt behavior).
 * - No 128-bit division/mod operators are used (only shifts/subtracts).
 */
u128 __udivmodti4(u128 dividend, u128 divisor, u128 *rem)
{
    if (divisor == 0)
    {
        if (rem)
            *rem = dividend;
        return ~((u128)0);
    }
    if (dividend == 0)
    {
        if (rem)
            *rem = 0;
        return 0;
    }
    if (divisor == 1)
    {
        if (rem)
            *rem = 0;
        return dividend;
    }
    if (dividend < divisor)
    {
        if (rem)
            *rem = dividend;
        return 0;
    }

    u128 q = 0;
    u128 r = 0;

    // Bitwise long division: iterate from MSB to LSB.
    for (int i = 127; i >= 0; --i)
    {
        // Bring down next bit
        r = (r << 1) | ((dividend >> i) & 1u);
        if (r >= divisor)
        {
            r -= divisor;
            q |= ((u128)1) << i;
        }
    }

    if (rem)
        *rem = r;
    return q;
}