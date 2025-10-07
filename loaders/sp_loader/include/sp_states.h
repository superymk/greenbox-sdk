// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/************ Management of IOSMs' States **********/

#ifndef _APP_SP_STATES_H
#define _APP_SP_STATES_H

#ifndef __ASSEMBLY__

typedef enum 
{
	SP_REGISTERED = 0,
	SP_RUNNING = 1,
	SP_EXITED = 2,

	SP_INIT_ERR = 3,

	SP_STATE_INVALID = 4, // invalid state for type safety purpose
} SP_STATE;

extern void sp_states_init(void);
extern void sp_states_fini(void);

//! Return true on success. Otherwise, return false
extern bool sp_states_add(spid_t sp_id, SP_STATE state);

extern SP_STATE sp_states_get(spid_t sp_id);

//! Return true when success. Otherwise, return false (e.g. sp_id not found)
extern bool sp_states_set(spid_t sp_id, SP_STATE state);

//! Return true on success. Otherwise, return false
extern bool sp_states_remove(spid_t sp_id);


#define SECSRV_INIT_TIMEOUT_MS (5000)
#define IOSM_EXIT_TIMEOUT_MS (5000)

extern bool sp_block_wait_init(spid_t sp_id, uint32_t timeout_ms);

extern bool sp_block_wait_exit(spid_t sp_id, uint32_t timeout_ms);

#endif // __ASSEMBLY__
#endif // _APP_SP_STATES_H

