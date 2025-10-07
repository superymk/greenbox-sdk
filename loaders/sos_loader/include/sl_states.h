// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECOS_LOADER_STATES_H
#define SECOS_LOADER_STATES_H

#ifndef __ASSEMBLY__

typedef enum 
{
	SECOS_UNLOADED = 0,
    SECOS_LOADED = 1,
    SECOS_REGISTERED = 2,
	SECOS_RUNNING = 3,

	SL_STATE_ANY = 98,
	SL_STATE_INVALID = 99, // invalid state for type safety purpose
} SL_STATE;

extern void sl_state_init(void);
extern void sl_state_fini(void);

/// @brief Transite from the state <state_from> to the state <state_to>. If the current state does not match the state 
/// <state_from>, then do nothing.
/// If <state_from> is SL_STATE_ANY, then the state transit always happens, even if the current state is 
/// SL_STATE_INVALID.
/// @param state_from 
/// @param state_to 
extern void sl_state_transit(SL_STATE state_from, SL_STATE state_to);

//! Update the current status of the SecOS loader
extern SL_STATE sl_state_get();

#endif // __ASSEMBLY__
#endif // SECOS_LOADER_STATES_H