// /*
//  * -------------------------------------------------------------------------------------
//  * Carnegie Mellon University
//  * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
//  * 
//  * This software is the proprietary information of Carnegie Mellon University.
//  * Use is subject to license terms.
//  * 
//  * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
//  * written consent of Carnegie Mellon University. is strictly prohibited.
//  * -------------------------------------------------------------------------------------
//  * 
//  * Authors: Miao Yu
//  *          Virgil Gligor
//  */

#include <appddk.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/eventfd.h>

#define SECAGENT_NOTIFY_WATCH_SELECT_FUNC_TIMEOUT   (5)

static pthread_t sa_notify_thread = 0;
static int efd = 0; //Eventfd file descriptor
static uint64_t last_read_eftd_ctr = 0;
static fd_set rfds = {0};

static bool watch_thread_run = true;

static void* _sa_notify_watch(void* data)
{
    uint64_t eftd_ctr = 0;
    int ret = 0;
    struct timeval tv;

    // GUI thread binds to core 2.
	app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_2);

    // Watch efd
    FD_ZERO(&rfds);
    FD_SET(efd, &rfds);

    //Wait up to <SECAGENT_NOTIFY_WATCH_SELECT_FUNC_TIMEOUT> seconds
    tv.tv_sec = SECAGENT_NOTIFY_WATCH_SELECT_FUNC_TIMEOUT;
    tv.tv_usec = 0;

    do
    {
        ret = select(efd+1, &rfds, NULL, NULL, &tv);
        if(ret >= 0)
        {
            ssize_t read_sz = 0;
            read_sz = read(efd, &eftd_ctr, sizeof(uint64_t));
            
            // [TODO][Issue 93] Not sure why <eftd_ctr> only changes once ...
            // if((read_sz == sizeof(uint64_t)) && 
            //     (eftd_ctr == last_read_eftd_ctr + 1)
            // )
            if((read_sz == sizeof(uint64_t)))
            {
                last_read_eftd_ctr = eftd_ctr;
                // Call the notification handler defined in the untrusted application
                on_secagent_notify();
            }
        }
    } while (watch_thread_run);

    return NULL;
}

bool app_sa_notify_init(void)
{
    OSSTATUS sa_status = OS_SUCCESS;
    struct sa_app_notif_reginfo reginfo = {0};

    // Create eventfd
    last_read_eftd_ctr = 0;
    efd = eventfd(last_read_eftd_ctr, 0);
    if (efd == -1)
    {
        APP_ERR(" <app_sa_notify_init> Failed to create the notification object!\n");
        goto err;
    }

    // Register the notification object in SecAgent
    reginfo.event_obj_info = (ulong_t)efd;
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
    if(efd != -1)
        close(efd);
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
    if(efd != -1)
        close(efd);

    // Clean the values
    sa_notify_thread = 0;
    efd = 0;
    last_read_eftd_ctr = 0;
    memset(&rfds, 0, sizeof(rfds));
}