// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sl_headers.h>

static SL_STATE current_state = SL_STATE_INVALID;
static LOADER_MLOCK_T sl_cur_state_lock;

/// @brief Update <current_state> and ensure cache coherence (memory propagation)
/// @param new_state
static void _sync_sl_state_set(SL_STATE new_state)
{
    (void)__sync_lock_test_and_set(&current_state, new_state);
}

/// @brief Get the current status of the SecOS loader
/// @param state 
static void sl_state_set(SL_STATE state)
{
    LOADER_ACQUIRE_LOCK(&sl_cur_state_lock);
    if ((state >= 0) && (state < SL_STATE_INVALID))
    {
        do
        {
            _sync_sl_state_set(state);
        } while (state != current_state);
    }

    LOADER_RELEASE_LOCK(&sl_cur_state_lock);
}

void sl_state_init(void)
{
    LOADER_CREATE_LOCK(&sl_cur_state_lock);
}

void sl_state_fini(void)
{
    LOADER_DESTROY_LOCK(&sl_cur_state_lock);
}

void sl_state_transit(SL_STATE state_from, SL_STATE state_to)
{
    if (state_from == SL_STATE_ANY)
    {
        // If <state_from> is SL_STATE_ANY, then the state transit always happens, even if the current state is
        // SL_STATE_INVALID.
        sl_state_set(state_to);
    }
    else
    {
        SL_STATE cur_state = sl_state_get();

        // Transite from the state <state_from> to the state <state_to> if the current state is <state_from>.
        if(cur_state == state_from)
        {
            sl_state_set(state_to);
        }
    }
}

SL_STATE sl_state_get(void)
{
    SL_STATE result;

    LOADER_ACQUIRE_LOCK(&sl_cur_state_lock);
    result = current_state;
    LOADER_RELEASE_LOCK(&sl_cur_state_lock);

    return result;
}