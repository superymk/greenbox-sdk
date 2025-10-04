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