// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>


/// @brief Spin lock store for <CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE>
static volatile uint32_t _lock_store[GLOBAL_SPINLOCK_MAX_SELECTOR] = {0};

static void _lock_store_clear(void)
{
    uint32_t i = 0;

    FOREACH_S(i, GLOBAL_SPINLOCK_MAX_SELECTOR, GLOBAL_SPINLOCK_MAX_SELECTOR, 0, 1)
    {
        _lock_store[i] = 0;
    }
}

/********* Public Functions *********/
OSSTATUS nsdom_utils_init(void)
{
    _lock_store_clear();

    // On success
    return OS_SUCCESS;
}

void nsdom_utils_fini(void)
{
    _lock_store_clear();
}

OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_TRY_ACQUIRE(struct tz_cmd_packet *cmd_packet)
{
    bool lock_got = false;

    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;

    if(!VALUE_IN_RANGE(cmd_packet->arg1, 0, GLOBAL_SPINLOCK_MAX_SELECTOR))
        return E_INV_ARG;

    if(_lock_store[cmd_packet->arg1] == 0)
    {
        // The lock is free
        os->atomic_increase((uint32_t*)&_lock_store[cmd_packet->arg1]);
        lock_got = true;
    }
    else
    {
        // The lock is not free
        lock_got = false;
    }

    cmd_packet->arg2 = (uint64_t)lock_got;

    // On success
    return OS_SUCCESS;
}

OSSTATUS do_CMD_NSDOM_UTILS_GLOBAL_SPINLOCK_RELEASE(struct tz_cmd_packet *cmd_packet)
{
    // Check: Valid params in <cmd_packet>
    if(!cmd_packet)
        return E_INV_ARG;

    if(!VALUE_IN_RANGE(cmd_packet->arg1, 0, GLOBAL_SPINLOCK_MAX_SELECTOR))
        return E_INV_ARG;

    if(_lock_store[cmd_packet->arg1] > 0)
    {
        // The lock can be released
        os->atomic_decrease((uint32_t*)&_lock_store[cmd_packet->arg1]);
    }

    // On success
    return OS_SUCCESS;
}