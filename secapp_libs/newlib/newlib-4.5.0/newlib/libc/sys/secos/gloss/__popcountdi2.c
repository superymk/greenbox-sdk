/* Portable reimplementation of LLVM's __popcountdi2 using standard C types.
 * No dependency on compiler-rt's int_lib.h typedefs.
 *
 * Signature matches the ABI: returns int, takes a 64-bit integer.
 * Using 'long long' for the parameter keeps the i64 calling convention,
 * and we cast to uint64_t to avoid any sign-shift pitfalls.
 */

#include <stdint.h>

int __popcountdi2(long long a)
{
    uint64_t x2 = (uint64_t)a;

    /* Subtract pairs: each 2-bit field becomes the count of its two bits */
    x2 = x2 - ((x2 >> 1) & UINT64_C(0x5555555555555555));

    /* Sum each 2-bit pair into each 4-bit field */
    x2 = ((x2 >> 2) & UINT64_C(0x3333333333333333)) +
         (x2 &        UINT64_C(0x3333333333333333));

    /* Now each 4-bit nibble holds its bit count (0..4) */
    x2 = (x2 + (x2 >> 4)) & UINT64_C(0x0F0F0F0F0F0F0F0F);

    /* Collapse to 32 bits by summing upper/lower halves */
    uint32_t x = (uint32_t)(x2 + (x2 >> 32));

    /* Fold to 16, then 8, keeping enough significant bits for the total (<=64) */
    x += x >> 16;
    x  = (x + (x >> 8)) & 0x7Fu;  /* 7 bits are enough for counts up to 64 */

    return (int)x;
}
