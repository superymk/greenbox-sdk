/*
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/opensslconf.h>

#include <openssl/rand.h>
#include "crypto/rand_pool.h"
#include "crypto/rand.h"
#include "internal/cryptlib.h"
#include "prov/seeding.h"

#include <sap_headers.h>
#include <palddk.h>



// #if defined(OPENSSL_RAND_SEED_NONE)
// /* none means none */
// # undef OPENSSL_RAND_SEED_OS
// #endif

// #if defined(OPENSSL_RAND_SEED_OS)
// # if _WRS_VXWORKS_MAJOR >= 7
// #   define RAND_SEED_VXRANDLIB
// # else
// #   error "VxWorks <7 only support RAND_SEED_NONE"
// # endif
// #endif

// #if defined(RAND_SEED_VXRANDLIB)
// # include <randomNumGen.h>
// #endif

static uint64_t get_time_stamp(void)
{
    return sp_perf_rdtscll();
}

/*
 * empty implementation
 * vxworks does not need to init/cleanup or keep open the random lib
 */
int ossl_rand_pool_init(void)
{
    return 1;
}

void ossl_rand_pool_cleanup(void)
{
}

void ossl_rand_pool_keep_random_devices_open(int keep)
{
}

int ossl_pool_add_nonce_data(RAND_POOL *pool)
{
    struct {
        spid_t pid;
        uint64_t time;
    } data;

    memset(&data, 0, sizeof(data));

    /*
     * Add process id, thread id, and a high resolution timestamp to
     * ensure that the nonce is unique with high probability for
     * different process instances.
     */
    data.pid = current_sp->sp_id;
    data.time = get_time_stamp();

    return ossl_rand_pool_add(pool, (unsigned char *)&data, sizeof(data), 0);
}

// size_t ossl_pool_acquire_entropy(RAND_POOL *pool)
// {
// #if defined(RAND_SEED_VXRANDLIB)
//     /* vxRandLib based entropy method */
//     size_t bytes_needed;

//     bytes_needed = ossl_rand_pool_bytes_needed(pool, 1 /*entropy_factor*/);
//     if (bytes_needed > 0)
//     {
//         int retryCount = 0;
//         STATUS result = ERROR;
//         unsigned char *buffer;

//         buffer = ossl_rand_pool_add_begin(pool, bytes_needed);
//         while ((result != OK) && (retryCount < 10)) {
//             RANDOM_NUM_GEN_STATUS status = randStatus();

//             if ((status == RANDOM_NUM_GEN_ENOUGH_ENTROPY)
//                     || (status == RANDOM_NUM_GEN_MAX_ENTROPY)) {
//                 result = randBytes(buffer, bytes_needed);
//                 if (result == OK)
//                     ossl_rand_pool_add_end(pool, bytes_needed, 8 * bytes_needed);
//                 /*
//                  * no else here: randStatus said ok, if randBytes failed
//                  * it will result in another loop or no entropy
//                  */
//             } else {
//                 /*
//                  * give a minimum delay here to allow OS to collect more
//                  * entropy. taskDelay duration will depend on the system tick,
//                  * this is by design as the sw-random lib uses interrupts
//                  * which will at least happen during ticks
//                  */
//                 taskDelay(5);
//             }
//             retryCount++;
//         }
//     }
//     return ossl_rand_pool_entropy_available(pool);
// #else
//     /*
//      * SEED_NONE means none, without randlib we dont have entropy and
//      * rely on it being added externally
//      */
//     return ossl_rand_pool_entropy_available(pool);
// #endif /* defined(RAND_SEED_VXRANDLIB) */
// }

#define SECOS_HW_PRNG_TRY_GET_SIZE (32)
static bool _sos_prng_try_get(void)
{
    VKSTATUS sos_status = SUCCESS;
    uint8_t random[SECOS_HW_PRNG_TRY_GET_SIZE] = {0};

    sos_status = sos_sys_hw_prng_get_random_number(SECOS_HW_PRNG_TRY_GET_SIZE, random);
    if(FAIL(sos_status))
    {
        printf("SecOS cannot return PRNG seeded by HW! status:%lu\n", sos_status);
        return false;
    }

    // On success
    return true;
}

/*
 * Acquire entropy from high-speed clock
 *
 * Since we get some randomness from the low-order bits of the
 * high-speed clock, it can help.
 *
 * Returns the total entropy count, if it exceeds the requested
 * entropy count. Otherwise, returns an entropy count of 0.
 */
size_t ossl_pool_acquire_entropy(RAND_POOL *pool)
{
    // Try to get PRNG from hardware first
    if(_sos_prng_try_get())
    {
        // Get PRNG from hardware
#define SECOS_HW_PRNG_GET_SIZE                 (256)
#define SECOS_BITS_PER_BYTE                     (8)
        VKSTATUS sos_status = SUCCESS;
        uint8_t random[SECOS_HW_PRNG_GET_SIZE] = {0};

        while((pool->entropy < pool->entropy_requested) || (pool->len < pool->min_len))
        {
            sos_status = sos_sys_hw_prng_get_random_number(SECOS_HW_PRNG_GET_SIZE, random);
            if(FAIL(sos_status))
            {
                printf("SecOS cannot return PRNG seeded by HW (requested bytes:%u)! status:%lu\n", SECOS_HW_PRNG_GET_SIZE, sos_status);
                goto acquire_via_rdtsc;
            }
            else
            {
                ossl_rand_pool_add(pool, random, SECOS_HW_PRNG_GET_SIZE, SECOS_HW_PRNG_GET_SIZE * SECOS_BITS_PER_BYTE);
            }
        }

        // Succeed to get the entropy from SecOS
        return ossl_rand_pool_entropy_available(pool);
    }

acquire_via_rdtsc:
    // Handle the case that SecOS cannot generate the requested random number.
    {
        // Get PRNG from rdtsc
#define TSC_READ_COUNT                 4
        unsigned char c;
        int i;

        // if ((OPENSSL_ia32cap_P[0] & (1 << 4)) != 0) 
        // {
        //     for (i = 0; i < TSC_READ_COUNT; i++) {
        //         c = (unsigned char)(OPENSSL_rdtsc() & 0xFF);
        //         ossl_rand_pool_add(pool, &c, 1, 4);
        //     }
        // }

        while((pool->entropy < pool->entropy_requested) || (pool->len < pool->min_len))
        {
            for (i = 0; i < TSC_READ_COUNT; i++)
            {
                c = (unsigned char)(get_time_stamp() & 0xFF);
                ossl_rand_pool_add(pool, &c, 1, 4);
            }

            // Long enough to trigger a process re-schedule.
            usleep(20 * 1000);
        }
    }

    return ossl_rand_pool_entropy_available(pool);
}


// if (pool->entropy < pool->entropy_requested)
//         return 0;

//     if (pool->len < pool->min_len)
//         return 0;

//     return pool->entropy;