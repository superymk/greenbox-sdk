// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_test.h>

#include <openssl/rand.h>

#define RAND_NUMBER_LEN     (16)

static bool _do_generate_rand_number(uint8_t* out_buffer, size_t len)
{
    // Generate random bytes using OpenSSL
    if (RAND_bytes(out_buffer, len) != 1) {
        SAP_TEST_ERR("Failed to generate random bytes\n");
        return false;
    }

    return true;
}

static bool _check_random_numbers_equal(uint8_t* rand1, uint8_t* rand2, size_t sz)
{
    uint32_t i = 0;

    print_hex("rand1 ", rand1, RAND_NUMBER_LEN);
    print_hex("rand2 ", rand2, RAND_NUMBER_LEN);

    FOREACH_S(i, sz, sz, 0, 1)
    {
        if(rand1[i] != rand2[i])
            return false;
    }

    return true;
}

bool test_openssl_generate_rand_number(void)
{
	bool ret = true;
    uint8_t rand1[RAND_NUMBER_LEN] = {0};
    uint8_t rand2[RAND_NUMBER_LEN] = {0};
    

    ret = _do_generate_rand_number(rand1, RAND_NUMBER_LEN);
    if(!ret)
        return false;

    ret = _do_generate_rand_number(rand2, RAND_NUMBER_LEN);
    if(!ret)
        return false;

    // The two random numbers must be different
    return !_check_random_numbers_equal(rand1, rand2, RAND_NUMBER_LEN);
}

bool test_openssl_get_hw_prng(void)
{
    VKSTATUS sos_status = SUCCESS;
    SECBASE_PLATFORM_NONSENSITIVE_INFO platform_info = {0};

    sos_status = sos_sys_platform_get_nonsensitive_info(&platform_info);
    if(FAIL(sos_status))
    {
        SAP_TEST_ERR("Failed to get platform info! status:%lu\n", sos_status);
        return false;
    }

    if(platform_info.enable_tpm)
    {
        uint8_t rand1[RAND_NUMBER_LEN] = {0};
        uint8_t rand2[RAND_NUMBER_LEN] = {0};

        sos_status = sos_sys_hw_prng_get_random_number(RAND_NUMBER_LEN, rand1);
        if(FAIL(sos_status))
        {
            printf("SecOS cannot return PRNG #1 seeded by HW (requested bytes:%u)! status:%lu\n", RAND_NUMBER_LEN, sos_status);
            return false;
        }

        sos_status = sos_sys_hw_prng_get_random_number(RAND_NUMBER_LEN, rand2);
        if(FAIL(sos_status))
        {
            printf("SecOS cannot return PRNG #2 seeded by HW (requested bytes:%u)! status:%lu\n", RAND_NUMBER_LEN, sos_status);
            return false;
        }

        // The two random numbers must be different
        return !_check_random_numbers_equal(rand1, rand2, RAND_NUMBER_LEN);
    }
    else
    {
        // No TPM. Return true;
        return true;
    }

    // Should not hit here
    return false;
}