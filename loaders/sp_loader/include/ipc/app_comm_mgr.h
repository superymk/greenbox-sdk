// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SECAPP_COMMUNICATION_MGR
#define APP_SECAPP_COMMUNICATION_MGR

#define COMM_SP_APP_ENTRY_NUM         (16)

#ifndef __ASSEMBLY__

// Forward declaration
struct _comm_sp_app;
typedef struct _comm_sp_app COMM_SP_APP;

extern void comm_app_sp_mgr_init(void);

extern void comm_app_sp_mgr_fini(void);

extern int comm_app_sp_register(spid_t sp_id, COMM_SP_APP* comm);

extern int comm_app_sp_unregister(spid_t sp_id);

extern COMM_SP_APP* comm_app_sp_get(spid_t sp_id);


#endif // __ASSEMBLY__
#endif // APP_SECAPP_COMMUNICATION_MGR