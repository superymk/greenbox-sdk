// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef TEST_OPENSSL_H
#define TEST_OPENSSL_H

/// @brief Generate two random numbers using Openssl's rand().
/// Check: they must be different.
/// @param  
/// @return 
extern bool test_openssl_generate_rand_number(void);

/// @brief If the platform has TPM, then generate two random numbers using SecOS's <SYS_HW_PRNG_GET_RANDOM_NUMBER>.
/// Check: If the platform has TPM, then the random numbers must be different. If no TPM, then the test passes.
/// @param  
/// @return 
extern bool test_openssl_get_hw_prng(void);

#endif // TEST_OPENSSL_H