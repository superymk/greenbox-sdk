/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#include <unistd.h>
#include <appddk.h>

/************ Management of IOSMs' States **********/
#define MAX_CONCURRENT_SPs    (SOS_MAX_SPs)       //!< The maximum allowed number of PALs at the same time.
struct sp_state_info
{
    spid_t sp_id;
    SP_STATE state;
};

static struct sp_state_info sp_states[MAX_CONCURRENT_SPs];
static LOADER_MLOCK_T states_lock;

/// @brief Update <sp_state_entry> (SP_STATE only) and ensure cache coherence (memory propagation)
/// @param new_state 
static void _sync_sp_state_entry_set_state_only(struct sp_state_info* sp_state_entry, SP_STATE new_state)
{
    (void)__sync_lock_test_and_set(&sp_state_entry->state, new_state);
}

/// @brief Update <sp_state_entry> and ensure cache coherence (memory propagation)
/// @param new_state 
static void _sync_sp_state_entry_set(struct sp_state_info* sp_state_entry, spid_t new_sp_id, SP_STATE new_state)
{
    (void)__sync_lock_test_and_set(&sp_state_entry->sp_id, new_sp_id);
    (void)__sync_lock_test_and_set(&sp_state_entry->state, new_state);
}

static bool find_free_siginfo_slot(uint32_t *out_idx)
{
    uint32_t i = 0;

    if(!out_idx)
        return false;

    for(i = 0; i < MAX_CONCURRENT_SPs; i++)
    {
        if(sp_states[i].sp_id == INVALID_SP_ID)
        {
            *out_idx = i;
            return true;
        }
    }

    return false;
}

void sp_states_init(void)
{
    uint32_t i = 0;

    LOADER_CREATE_LOCK(&states_lock);

    LOADER_ACQUIRE_LOCK(&states_lock);
    for (i = 0; i < MAX_CONCURRENT_SPs; i++)
    {
        _sync_sp_state_entry_set(&sp_states[i], INVALID_SP_ID, SP_STATE_INVALID);
    }
    LOADER_RELEASE_LOCK(&states_lock);
}

void sp_states_fini(void)
{
    uint32_t i = 0;

    LOADER_ACQUIRE_LOCK(&states_lock);
    for (i = 0; i < MAX_CONCURRENT_SPs; i++)
    {
        _sync_sp_state_entry_set(&sp_states[i], INVALID_SP_ID, SP_STATE_INVALID);
    }
    LOADER_RELEASE_LOCK(&states_lock);

    LOADER_DESTROY_LOCK(&states_lock);
}


//! Return true on success. Otherwise, return false.
bool sp_states_add(spid_t sp_id, SP_STATE state)
{
    bool has_free_slot = false;
    uint32_t slot_idx = 0;

    // Check: input params must be valid
    if(sp_id == INVALID_SP_ID || state == SP_STATE_INVALID)
        return false;

    // Check: sp_id must have not already been registered.
    if(sp_states_get(sp_id) != SP_STATE_INVALID)
        return false;

    // Record the state of the given SecProcess
    LOADER_ACQUIRE_LOCK(&states_lock);
    has_free_slot = find_free_siginfo_slot(&slot_idx);
    if(!has_free_slot)
    {
        APP_ERR("[SecProcess State Manager] Too many active SecProcesses!\n");
        LOADER_RELEASE_LOCK(&states_lock);
        return false;
    }

    // Record
    _sync_sp_state_entry_set(&sp_states[slot_idx], sp_id, state);
    LOADER_RELEASE_LOCK(&states_lock);

    return true;
}

SP_STATE sp_states_get(spid_t sp_id)
{
    uint32_t i = 0;

    LOADER_ACQUIRE_LOCK(&states_lock);
    for(i = 0; i < MAX_CONCURRENT_SPs; i++)
    {
        if(sp_states[i].sp_id == sp_id)
        {
            SP_STATE result = sp_states[i].state;
            LOADER_RELEASE_LOCK(&states_lock);
            return result;
        }
    }
    LOADER_RELEASE_LOCK(&states_lock);

    return SP_STATE_INVALID;
}

//! Return true when success. Otherwise, return false (e.g. sp_id not found)
bool sp_states_set(spid_t sp_id, SP_STATE state)
{
    uint32_t i = 0;

    LOADER_ACQUIRE_LOCK(&states_lock);
    for(i = 0; i < MAX_CONCURRENT_SPs; i++)
    {
        if(sp_states[i].sp_id == sp_id)
        {
            _sync_sp_state_entry_set_state_only(&sp_states[i], state);
            LOADER_RELEASE_LOCK(&states_lock);
            return true;
        }
    }
    LOADER_RELEASE_LOCK(&states_lock);

    return false;
}

//! Return true on success. Otherwise, return false
bool sp_states_remove(spid_t sp_id)
{
    uint32_t i = 0;

    LOADER_ACQUIRE_LOCK(&states_lock);
    for(i = 0; i < MAX_CONCURRENT_SPs; i++)
    {
        if(sp_states[i].sp_id == sp_id)
        {
            _sync_sp_state_entry_set(&sp_states[i], INVALID_SP_ID, SP_STATE_INVALID);
            LOADER_RELEASE_LOCK(&states_lock);
            return true;
        }
    }
    LOADER_RELEASE_LOCK(&states_lock);

    return false;
}

#define BLOCK_WAIT_MSLEEP_PER_ROUND (100)

bool sp_block_wait_init(spid_t sp_id, uint32_t timeout_ms)
{
    volatile SP_STATE state;
    volatile uint32_t timeout_round = timeout_ms / BLOCK_WAIT_MSLEEP_PER_ROUND;

    do
    {
        state = sp_states_get(sp_id);

        if (state == SP_RUNNING)
            return true;
        else if (state == SP_INIT_ERR)
        {
            // Init error
            return false; 
        }
        else if (state == SP_EXITED)
        {
            // SecService exits and overwrites the <SP_INIT_ERR> status before the loader is able to read it.
            return false;
        }

        mdelay(BLOCK_WAIT_MSLEEP_PER_ROUND);
        timeout_round--;
    } while (timeout_round != 0);

    return false; // Timeout return, so return error
}

bool sp_block_wait_exit(spid_t sp_id, uint32_t timeout_ms)
{
    volatile SP_STATE state;
    volatile uint32_t timeout_round = timeout_ms / BLOCK_WAIT_MSLEEP_PER_ROUND;

    do
    {
        state = sp_states_get(sp_id);
        if (state == SP_EXITED)
            return true;

        mdelay(BLOCK_WAIT_MSLEEP_PER_ROUND);
        timeout_round--;
    } while (timeout_round != 0);

    return false; // Timeout return, so return error
}