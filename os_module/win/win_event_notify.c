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

#include <ntddk.h>
#include <wdm.h>
#include <sa_headers.h>



#include "win_event_notify.h"

#define OS_APP_EVENT_SLOTS		(64)

struct app_event_map
{
	process_id  pid;
    PKEVENT     event;
};

static struct app_event_map app_event_map[OS_APP_EVENT_SLOTS] = {{0}};

static void _win_os_app_notif_map_clear(void)
{
	uint32_t i = 0;

	FOREACH_S(i, OS_APP_EVENT_SLOTS, OS_APP_EVENT_SLOTS, 0, 1)
	{
		app_event_map[i].pid = INVALID_UNTRUSTED_PROC_ID;
		app_event_map[i].event = NULL;
	}
}

static struct app_event_map* _win_os_app_notif_map_find(process_id pid)
{
	uint32_t i = 0;

	FOREACH_S(i, OS_APP_EVENT_SLOTS, OS_APP_EVENT_SLOTS, 0, 1)
	{
		if(app_event_map[i].pid == pid)
			return &app_event_map[i];
	}

	return NULL;
}




/********* Public Functions *********/
void win_os_app_notif_subsystem_init(void)
{
	_win_os_app_notif_map_clear();
}

void win_os_app_notif_subsystem_fini(void)
{
	_win_os_app_notif_map_clear();
}

OSSTATUS win_os_app_notif_register(struct sa_app_notif_reginfo* info)
{
    NTSTATUS win_status;
	process_id cur_pid = INVALID_UNTRUSTED_PROC_ID;
	PKEVENT event = NULL;
    HANDLE  h_inEvent;
	struct app_event_map* entry = NULL;

	if(!info)
		return E_INV_ARG;

	// Get the ID and the event object of the current process 
	cur_pid = os->get_current_pid();

    //
    // Get the object pointer from the handle. Note we must be in the context
    // of the process that created the handle.
    //
    h_inEvent = (HANDLE)info->event_obj_info;
    win_status = ObReferenceObjectByHandle(h_inEvent,
                                       SYNCHRONIZE | EVENT_MODIFY_STATE,
                                       *ExEventObjectType,
                                       UserMode,
                                       &event,
                                       NULL
                                      );

    if (!NT_SUCCESS(win_status)) 
    {
        SACOMM_ERR("<win_os_app_notif_register> Failed to get the event object (%u) of Process (%u)\n", h_inEvent, cur_pid);
        return E_INTERNAL_ERROR;
    }

	// Register
	//// No double registration
	entry = _win_os_app_notif_map_find(cur_pid);
	if(entry)
	{
		SACOMM_ERR("<win_os_app_notif_register> Process (%u) has registered an event!\n", cur_pid);
		return E_INV_ARG;
	}

	entry = _win_os_app_notif_map_find(INVALID_UNTRUSTED_PROC_ID);
	if(!entry)
	{
		SACOMM_ERR("<win_os_app_notif_register> No space to register Process (%u)'s event!\n", cur_pid);
		return E_INTERNAL_ERROR;
	}

	entry->pid = cur_pid;
	entry->event = event;

	// On success
	return OS_SUCCESS;
}

void win_os_app_notif_unregister(void)
{
	process_id cur_pid = INVALID_UNTRUSTED_PROC_ID;
	struct app_event_map* entry = NULL;

	cur_pid = os->get_current_pid();

	entry = _win_os_app_notif_map_find(cur_pid);
	if(entry)
	{
		if(entry->event)
            ObDereferenceObject(entry->event);

		entry->pid = INVALID_UNTRUSTED_PROC_ID;
		entry->event = NULL;
	}
}

#define EVENTFD_PLUS_ONE	(1)
void win_signal_app(process_id pid)
{
	struct app_event_map* entry = NULL;

	entry = _win_os_app_notif_map_find(pid);
	if(!entry)
	{
		SACOMM_ERR("<win_signal_app> Process (%u) has not registered an event to notify!\n", pid);
		return;
	}
	
	// Notify the event
    (void)KeSetEvent(entry->event, EVENTFD_PLUS_ONE, FALSE);

	SACOMM_DEBUG("<win_signal_app> Signaled Process (%u)!\n", pid);
}