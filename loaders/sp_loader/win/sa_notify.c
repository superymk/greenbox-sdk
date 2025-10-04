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

#include <appddk.h>
#include <windows.h>
#include <winioctl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define SECAGENT_NOTIFY_WATCH_SELECT_FUNC_TIMEOUT   (5)

static pthread_t sa_notify_thread = 0;
static HANDLE hEvent = NULL; // event handle

static bool watch_thread_run = true;

static void* _sa_notify_watch(void* data)
{
    DWORD wait_result = 0;

    // GUI thread binds to core 2.
	app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_2);

    do
    {
        // Wait up to <SECAGENT_NOTIFY_WATCH_SELECT_FUNC_TIMEOUT> seconds
        wait_result = WaitForSingleObject(hEvent, SECAGENT_NOTIFY_WATCH_SELECT_FUNC_TIMEOUT * 1000);
        if(wait_result == WAIT_OBJECT_0)
        {
            // The event is signaled
            // Call the notification handler defined in the untrusted application
            on_secagent_notify();

            // Clear the event
            ResetEvent(hEvent);
        }
    } while (watch_thread_run);

    return NULL;
}

bool app_sa_notify_init(void)
{
    OSSTATUS sa_status = OS_SUCCESS;
    struct sa_app_notif_reginfo reginfo = {0};

    // Create eventfd
    // CreateEvent returns NULL on errors
    hEvent = CreateEvent(NULL,   // lpEventAttributes
                        TRUE,   // bManualReset
                        FALSE,  // bInitialState
                        "SPLoader_Event"
                        );
    if(!hEvent)
    {
        APP_ERR(" <app_sa_notify_init> Failed to create the notification object!\n");
        goto err;
    }

    // Register the notification object in SecAgent
    reginfo.event_obj_info = (ulong_t)hEvent;
    sa_status = secagent_register_app_notification(&reginfo);
    if(SA_FAIL(sa_status))
    {
        APP_ERR(" <app_sa_notify_init> Failed to register the notification object in SecAgent!\n");
        goto err;
    }

    // Create watch thread
    watch_thread_run = true;
    pthread_create(&sa_notify_thread, NULL, _sa_notify_watch, NULL);

    // On success
    return true;

err:
    if(hEvent)
        CloseHandle(hEvent);
    return false;
}

void app_sa_notify_fini(void)
{
    OSSTATUS sa_status = OS_SUCCESS;

    // Stop the watch thread
    watch_thread_run = false;

    // Unregister the notification object in SecAgent
    sa_status = secagent_unregister_app_notification();
    if(SA_FAIL(sa_status))
    {
        APP_ERR(" <app_sa_notify_fini> Failed to unregister the notification object in SecAgent!\n");
    }

    // Close the access to the notification object
    if(hEvent)
        CloseHandle(hEvent);

    // Clean the values
    sa_notify_thread = 0;
    hEvent = NULL;
}