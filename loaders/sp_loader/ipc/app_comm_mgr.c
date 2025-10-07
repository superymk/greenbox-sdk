// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#include <ipc/app_comm.h>
#include <ipc/app_comm_ops.h>

extern void _comm_app_sp_fini(COMM_SP_APP *comm);


struct comm_app_sp_entry
{
    spid_t sp_id;
    
    COMM_SP_APP* comm;
};

static struct comm_app_sp_entry comm_map[COMM_SP_APP_ENTRY_NUM];

/********* Private Functions *********/
static void _clear_entries(void)
{
    uint32_t i = 0;
    FOREACH_S(i, COMM_SP_APP_ENTRY_NUM, COMM_SP_APP_ENTRY_NUM, 0, 1)
    {
        struct comm_app_sp_entry* entry = &comm_map[i];

        entry->sp_id = INVALID_SP_ID;
        entry->comm = NULL;
    }
}

static int _find_entry(spid_t sp_id)
{
    uint32_t i = 0;
    FOREACH_S(i, COMM_SP_APP_ENTRY_NUM, COMM_SP_APP_ENTRY_NUM, 0, 1)
    {
        struct comm_app_sp_entry* entry = &comm_map[i];

        if(entry->sp_id == sp_id)
            return i;
    }

    return -1;
}

/// @brief Register a new SecProcess-App communication channel.
/// @param sp_id 
/// @param comm 
/// @return 
static int _comm_app_sp_register_new(spid_t sp_id, COMM_SP_APP* comm)
{
    int slot = 0;

    slot = _find_entry(INVALID_SP_ID);
    if(slot == -1)
    {
        // <comm_map> is full. Return error
        return 1;
    }

    // Register the new communication channel
    comm_map[slot].sp_id = sp_id;
    comm_map[slot].comm = comm;

    // On success
    return 0;
}




/********* Public Functions *********/
void comm_app_sp_mgr_init(void)
{
    _clear_entries();
}

void comm_app_sp_mgr_fini(void)
{
    uint32_t i = 0;

    // Finalize all SecProcess-App communications
    FOREACH_S(i, COMM_SP_APP_ENTRY_NUM, COMM_SP_APP_ENTRY_NUM, 0, 1)
    {
        struct comm_app_sp_entry* entry = &comm_map[i];

        if(entry->comm)
        {
            _comm_app_sp_fini(entry->comm);
        }
    }

    // Clear all records
    _clear_entries();
}

int comm_app_sp_register(spid_t sp_id, COMM_SP_APP* comm)
{
    int slot = 0;

    // Check: Parameters must be valid
    if(!comm)
        return 1;
    if(sp_id == INVALID_SP_ID)
        return 1;

    // Register the communication channel.
    slot = _find_entry(sp_id);
    if(slot == -1)
    {
        return _comm_app_sp_register_new(sp_id, comm);
    }
    else
    {
        // App already has a channel to the SecProcess <sp_id>
        struct comm_app_sp_entry* entry = &comm_map[slot];

        if(comm == entry->comm)
        {
            // Do nothing on re-registration
            return 0;
        }
        else
        {
            APPCOMM_ERR("Register SecProcess-App communication channel error: Already registered a different channel!\n");
            return 1;
        }
    }

    // Should not hit
    return 1;
}

int comm_app_sp_unregister(spid_t sp_id)
{
    int slot = 0;

    // Check: Parameters must be valid
    if(sp_id == INVALID_SP_ID)
        return 1;

    slot = _find_entry(sp_id);
    if(slot == -1)
    {
        // The SecProcess-App channel does not exist. Return error.
        return 1;
    }

    comm_map[slot].sp_id = INVALID_SP_ID;
    comm_map[slot].comm = NULL;

    // On success
    return 0;
}

COMM_SP_APP* comm_app_sp_get(spid_t sp_id)
{
    int slot = 0;

    // Check: Parameters must be valid
    if(sp_id == INVALID_SP_ID)
        return NULL;

    slot = _find_entry(sp_id);
    if(slot == -1)
    {
        // The SecProcess-App channel does not exist.
        return NULL;
    }

    // On success
    return comm_map[slot].comm;
}