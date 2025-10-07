// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/sched.h>	//find_task_by_pid_type
#include <linux/slab.h>

#include <linux/version.h>
#include <linux/fdtable.h>
#include <linux/eventfd.h>

#include "linux_event_notify.h"

#define OS_APP_EVENT_SLOTS		(64)

struct app_event_map
{
	process_id pid;
	struct eventfd_ctx* efd_ctx;
};

static struct app_event_map app_event_map[OS_APP_EVENT_SLOTS] = {{0}};

static void _linux_os_app_notif_map_clear(void)
{
	uint32_t i = 0;

	FOREACH_S(i, OS_APP_EVENT_SLOTS, OS_APP_EVENT_SLOTS, 0, 1)
	{
		app_event_map[i].pid = INVALID_UNTRUSTED_PROC_ID;
		app_event_map[i].efd_ctx = NULL;
	}
}

static struct app_event_map* _linux_os_app_notif_map_find(process_id pid)
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
void linux_os_app_notif_subsystem_init(void)
{
	_linux_os_app_notif_map_clear();
}

void linux_os_app_notif_subsystem_fini(void)
{
	_linux_os_app_notif_map_clear();
}

OSSTATUS linux_os_app_notif_register(struct sa_app_notif_reginfo* info)
{
	int efd = 0;
	process_id cur_pid = INVALID_UNTRUSTED_PROC_ID;
	struct eventfd_ctx* efd_ctx = NULL;
	struct app_event_map* entry = NULL;

	if(!info)
		return E_INV_ARG;

	// Get the ID and the event object of the current process 
	efd = (int)info->event_obj_info;
	cur_pid = os->get_current_pid();
	efd_ctx = eventfd_ctx_fdget(efd);

	if (IS_ERR(efd_ctx)) 
	{
		SACOMM_ERR("<linux_os_app_notif_register> Failed to get the event object (%d) of Process (%lu)\n", efd, cur_pid);
		return E_INTERNAL_ERROR;
	}

	// Register
	//// No double registration
	entry = _linux_os_app_notif_map_find(cur_pid);
	if(entry)
	{
		SACOMM_ERR("<linux_os_app_notif_register> Process (%lu) has registered an event!\n", cur_pid);
		return E_INV_ARG;
	}

	entry = _linux_os_app_notif_map_find(INVALID_UNTRUSTED_PROC_ID);
	if(!entry)
	{
		SACOMM_ERR("<linux_os_app_notif_register> No space to register Process (%lu)'s event!\n", cur_pid);
		return E_INTERNAL_ERROR;
	}

	entry->pid = cur_pid;
	entry->efd_ctx = efd_ctx;

	// On success
	return OS_SUCCESS;
}

void linux_os_app_notif_unregister(void)
{
	process_id cur_pid = INVALID_UNTRUSTED_PROC_ID;
	struct app_event_map* entry = NULL;

	cur_pid = os->get_current_pid();

	entry = _linux_os_app_notif_map_find(cur_pid);
	if(entry)
	{
		if(entry->efd_ctx)
			eventfd_ctx_put(entry->efd_ctx);

		entry->pid = INVALID_UNTRUSTED_PROC_ID;
		entry->efd_ctx = NULL;
	}
}

#define EVENTFD_PLUS_ONE	(1)
void linux_signal_app(process_id pid)
{
	int ret = 0;
	struct app_event_map* entry = NULL;

	entry = _linux_os_app_notif_map_find(pid);
	if(!entry)
	{
		SACOMM_ERR("<linux_signal_app> Process (%lu) has not registered an event to notify!\n", pid);
		return;
	}
	
	// Notify the event
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,7,12)
    eventfd_signal(entry->efd_ctx);
    ret = EVENTFD_PLUS_ONE;
#else
	ret = eventfd_signal(entry->efd_ctx, EVENTFD_PLUS_ONE);
#endif
	if(ret != EVENTFD_PLUS_ONE)
	{
		SACOMM_ERR("<linux_signal_app> Failed to notify Process (%lu)! ret:%d\n", pid, ret);
		return;
	}

	SACOMM_DEBUG("<linux_signal_app> Signaled Process (%lu)!\n", pid);
}