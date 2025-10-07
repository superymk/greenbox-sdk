// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/time.h>    // for gettimeofday()
#include <unistd.h>
#include <pthread.h>
#include <machine/i386/atomic.h>

#include <ipc/app_comm.h>
#include <ipc/app_comm_ops.h>
#include "app_event_handler.h"

#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

/// @brief If the backend thread receives a valid message from the SecProcess, the thread keeps polling in the next
/// 30 ms because the SecProcess are likely to request the backend thread more.
#define COMM_APP_SP_BUSY_POLLING_MILLISECONDS_AFTER_RECEIVING_MSG   (30)

/// @brief If the backend thread goes to the idle state (e.g., due to not receiving any messages within 
/// <COMM_APP_SP_BUSY_POLLING_MILLISECONDS_AFTER_RECEIVING_MSG> milliseconds), the number of milliseconds (at least) 
/// that the thread could be idle.
#define COMM_APP_SP_IDLE_INTERVAL_MILLISECONDS   (10)

extern bool cache_flush_range(vaddr_t vaddr, size_t size);
static void comm_app_sp_dump(COMM_SP_APP *comm);

static bool _comm_app_sp_thread_dtm = false;

static uint32_t _comm_ctl_get_p_slot(volatile struct ipc_ctl* comm_ctl)
{
	(void)cache_flush_range((vaddr_t)&comm_ctl->p_slot, sizeof(comm_ctl->p_slot));

	return comm_ctl->p_slot;
}

static void _comm_ctl_set_p_slot(volatile struct ipc_ctl* comm_ctl, uint32_t new_p_slot)
{
	comm_ctl->p_slot = new_p_slot;
	(void)cache_flush_range((vaddr_t)&comm_ctl->p_slot, sizeof(comm_ctl->p_slot));
}

static uint32_t _comm_ctl_get_c_slot(volatile struct ipc_ctl* comm_ctl)
{
	(void)cache_flush_range((vaddr_t)&comm_ctl->c_slot, sizeof(comm_ctl->c_slot));

	return comm_ctl->c_slot;
}

static void _comm_ctl_set_c_slot(volatile struct ipc_ctl* comm_ctl, uint32_t new_c_slot)
{
	comm_ctl->c_slot = new_c_slot;
	(void)cache_flush_range((vaddr_t)&comm_ctl->c_slot, sizeof(comm_ctl->c_slot));
}

static uint32_t _rb_increment(uint32_t val)
{
    atomic_increase(&val);

    return val % IPC_CHAN_NUM_MSG_SLOTS;
}

static bool _copy_to_comm_buf(volatile struct ipc_msg *comm_buf, uint32_t p_slot, struct ipc_msg *in_msg, uint32_t msg_size)
{
    if (p_slot >= IPC_CHAN_NUM_MSG_SLOTS)
        return false;

    memcpy((void *)&comm_buf[p_slot], in_msg, msg_size);
    (void)cache_flush_range((vaddr_t)&comm_buf[p_slot], msg_size);
    return true;
}

static bool _copy_from_comm_buf(volatile struct ipc_msg *comm_buf, uint32_t c_slot, void *buf, uint32_t buf_size)
{
    if (c_slot >= IPC_CHAN_NUM_MSG_SLOTS)
        return false;

    (void)cache_flush_range((vaddr_t)&comm_buf[c_slot], buf_size);
    memcpy(buf, (void *)&comm_buf[c_slot], buf_size);
    return true;
}

static bool _try_rb_enqueue(struct comm_secapp *comm_sp, struct ipc_msg *in_msg)
{
    // OSSTATUS sa_sts = OS_SUCCESS;
    bool sa_sts = true;
    volatile struct ipc_msg *comm_buf = NULL;
    volatile struct ipc_ctl *comm_ctl = NULL;
    struct comm_ipc_channel *chan = &comm_sp->send_chan;
    uint32_t p_slot = 0;
	uint32_t c_slot = 0;
    uint32_t next_p_slot = 0;
    uint32_t msg_size = 0;

    comm_ctl = CHAN_SAP_CTL(chan);
    comm_buf = CHAN_SAP_BUF(chan);

    // Get current producer counter
    p_slot = _comm_ctl_get_p_slot(comm_ctl);
	next_p_slot = _rb_increment(p_slot);
	c_slot = _comm_ctl_get_c_slot(comm_ctl);

    if (next_p_slot != c_slot)
    {
        // Set msg_size
        msg_size = IPC_MSG_HEADER_SIZE + in_msg->header.data_size;
        msg_size = min(msg_size, IPC_MSG_SIZE);

        // Commit message
        sa_sts = _copy_to_comm_buf(comm_buf, p_slot, in_msg, msg_size); // [TODO][Ticket 120] We do not need to copy it here.
        if (!sa_sts)
            goto out;

        // Increase the producer counter
        mfence();
        _comm_ctl_set_p_slot(comm_ctl, next_p_slot);

        // Update perf counters
        atomic_increase(&chan->counter_msgs);

        // // Flush the memory cache of the application side to make sure the data is written to memory
        // sa_sts = cache_flush_range((vaddr_t)comm_ctl, sizeof(struct ipc_ctl));
        // if (!sa_sts)
        //     APPCOMM_ERR("<_try_rb_enqueue> Failed to flush cache of comm_ctl!\n");
        // sa_sts = cache_flush_range((vaddr_t)comm_buf, sizeof(struct ipc_msg));
        // if (!sa_sts)
        //     APPCOMM_ERR("<_try_rb_enqueue> Failed to flush cache of comm_buf!\n");

        APPCOMM_DEBUG("<_try_rb_enqueue> c_slot:%u p_slot:%u\n",
                      comm_ctl->c_slot, comm_ctl->p_slot);

        return true;
    }

out:
    return false;
}

static bool _try_rb_dequeue(struct comm_secapp *comm_sp, struct ipc_msg *out_msg)
{
    // OSSTATUS sa_sts = OS_SUCCESS;
    bool sa_sts = true;
    volatile struct ipc_msg *comm_buf = NULL;
    volatile struct ipc_ctl *comm_ctl = NULL;
    struct comm_ipc_channel *chan = &comm_sp->recv_chan;
    uint32_t p_slot = 0;
	uint32_t c_slot = 0;

    comm_ctl = CHAN_SAP_CTL(chan);
    comm_buf = CHAN_SAP_BUF(chan);

    // // Flush the memory cache of the application side to make sure the data is freshly read from memory
    // sa_sts = cache_flush_range((vaddr_t)comm_ctl, sizeof(struct ipc_ctl));
    // if (!sa_sts)
    //     APPCOMM_ERR("<_try_rb_dequeue> Failed to flush cache of comm_ctl!\n");
    // sa_sts = cache_flush_range((vaddr_t)comm_buf, sizeof(struct ipc_msg));
    // if (!sa_sts)
    //     APPCOMM_ERR("<_try_rb_dequeue> Failed to flush cache of comm_buf!\n");

    // Get current consumer counter
    p_slot = _comm_ctl_get_p_slot(comm_ctl);
	c_slot = _comm_ctl_get_c_slot(comm_ctl);

    if (c_slot != p_slot)
    {
        struct ipc_header msg_header;
        uint32_t msg_size = 0;

        // Step 1. Inspect message header
        memset(&msg_header, 0, sizeof(msg_header)); // [NOTE] Use memset to initialize <msg_header> instead of initializing
                                                    // it in the local variable, as gcc may allocate non 16-byte aligned space for <msg_header> and use
                                                    // <movaps> instruction.
        sa_sts = _copy_from_comm_buf(comm_buf, c_slot, &msg_header, IPC_MSG_HEADER_SIZE);
        if (!sa_sts)
            goto out;

        msg_size = IPC_MSG_HEADER_SIZE + msg_header.data_size;
        msg_size = min(msg_size, IPC_MSG_SIZE); // [TODO] need to implement message split later

        // Step 2. Read the whole message (w/ the header)
        sa_sts = _copy_from_comm_buf(comm_buf, c_slot, out_msg, msg_size); // [TODO][Ticket 120] We do not need to copy it here.
        if (!sa_sts)
            goto out;

        // Increase the consumer counter
        mfence();
        _comm_ctl_set_c_slot(comm_ctl, _rb_increment(c_slot));

        // Update perf counters
        atomic_increase(&chan->counter_msgs);

        APPCOMM_DEBUG("<_try_rb_dequeue> &comm_buf[c_slot]:%p, c_slot:%u, "
                      "msg_size:%u\n",
                      TO_PTR((vaddr_t)&comm_buf[c_slot]), c_slot, msg_size);

        return true;
    }

out:
    return false;
}

/// @brief All backend threads needs to yield CPU resources if the DesktopMgr backend thread needs high priority; e.g., 
/// due to users ask DesktopMgr to forward keypresses to untrusted OS/Apps. Failure to do so causes the keyboard/mouse 
/// laggy in the untrusted OS/Apps.
/// @param out_dtm_need_high_priority Return true if the DesktopMgr backend thread needs high priority.
/// @return 
static int _dtm_thread_get_priority(bool* out_dtm_need_high_priority)
{
    OSSTATUS sa_status = OS_SUCCESS;
    uint64_t data = 0;

    sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DTM_BACKEND_THREAD_FORCE_PRIORITY, &data);
    if(SA_FAIL(sa_status))
    {
        APP_ERR("Failed to share desktop_mgr parameters with other NS processes! SecAgent status:%d\n", sa_status);
        return 1;
    }

    *out_dtm_need_high_priority = false;
    if(data == DTM_BACKEND_THREAD_PRIORITY_HIGHEST)
        *out_dtm_need_high_priority = true;
    else
        *out_dtm_need_high_priority = false;

    // On success
    return 0;
}

/// @brief Yield CPU for at least <COMM_APP_SP_IDLE_INTERVAL_MILLISECONDS> ms.
/// @param  
static void _comm_app_sp_thread_yield(void)
{
    // Put a sleep in order to yield CPU
    //// [NOTE] sched_yield() does not reduce CPU usage somehow...
    msleep(COMM_APP_SP_IDLE_INTERVAL_MILLISECONDS);
}

/// @brief Handle received SecApp msgs.
/// @param data
/// @return
static void *_comm_app_sp_on_msg_arrival(void *data)
{
    int ret = 0;
    COMM_SP_APP *comm = (COMM_SP_APP *)data;
    struct comm_secapp *comm_sp = NULL;
    struct ipc_msg *msg = NULL;
    bool msg_ready = false;
    struct timeval last_valid_time = {0, 0};

    app_set_cpu_affinity(SP_LOADER_CPU_AFFINITY_CORE_2);

    // Check: params must be valid
    if (!comm)
        return NULL;
    comm_sp = &comm->comm;

    msg = (struct ipc_msg *)malloc(sizeof(struct ipc_msg));
    if (!msg)
    {
        APPCOMM_ERR("Handle message error: Allocate message buffer error!\n");
        return NULL;
    }

    // Handle received SecApp msgs
    do
    {
        msg_ready = _try_rb_dequeue(comm_sp, msg);
        if (msg_ready)
        {
            bool status = true;
            bool is_valid_msg = false;

            status = ipc_handle_msg(comm, msg, &is_valid_msg);
            if (!status)
            {
                APPCOMM_ERR("Handle message error: handle error on message:%u!\n", msg->header.req_id);
            }

            // If the backend thread receives a valid message from the SecProcess, the thread keeps polling in the next
            // tens of milliseconds because the SecProcess are likely to request the backend thread more.
            if(is_valid_msg)
            {
                // Record the time we got a valid message and enter the constant‐polling window.
                gettimeofday(&last_valid_time, NULL);
            }
        }
        else
        {
            // Busy-poll or yield CPU for next messages.
            if(_comm_app_sp_thread_dtm)
            {
                // If this thread is the DesktopMgr backend thread and DesktopMgr demands high priority; e.g., sending
                // keypresses to untrusted OS/Apps. Then the thread must busy-poll for next messages.
                bool dtm_need_high_priority = false;

                ret = _dtm_thread_get_priority(&dtm_need_high_priority);
                if(!ret && dtm_need_high_priority)
                {
                    //// A conservative approach: If failed to get its priority, the DesktopMgr backend thread yield CPU.
                    continue;
                }
                else
                {
                    // Otherwise, yield CPU.
                    _comm_app_sp_thread_yield();
                }
            }
            else
            {
                // If this thread is not the DesktopMgr backend thread, then busy-poll for the next message in 
                // <COMM_APP_SP_BUSY_POLLING_MILLISECONDS_AFTER_RECEIVING_MSG> milliseconds. If no message arrives, then
                // yield CPU.
                struct timeval now;
                long elapsed_ms;

                gettimeofday(&now, NULL);
                elapsed_ms = (now.tv_sec  - last_valid_time.tv_sec)  * 1000
                            + (now.tv_usec - last_valid_time.tv_usec) / 1000;

                if (elapsed_ms < COMM_APP_SP_BUSY_POLLING_MILLISECONDS_AFTER_RECEIVING_MSG)
                {
                    bool dtm_need_high_priority = false;

                    ret = _dtm_thread_get_priority(&dtm_need_high_priority);
                    if(ret || dtm_need_high_priority)
                    {
                        // Yield CPU if DesktopMgr backend thread needs high priority.
                        //// A conservative approach: If failed to get DesktopMgr backend thread's priority, other 
                        //// backend threads yield CPU.
                        _comm_app_sp_thread_yield();
                    }
                    else
                    {
                        // Otherwise, if still within the constant‐polling window, busy‐poll.
                        continue;
                    }
                }
                else
                {
                    // Otherwise, yield CPU.
                    _comm_app_sp_thread_yield();
                }
            }
        }
    } while (sync_read_bool(&comm->is_enabled));

    free(msg);

    sync_write_bool(&comm->is_app_handler_terminated, true);

    return NULL;
}

static void _do_comm_app_sp_init(SP_USTRUCT *params, COMM_SP_APP *out_comm)
{
    struct comm_secapp *comm_sp = &out_comm->comm;
    pthread_t event_thread;

    out_comm->sp_id = params->sp_id;

    sync_write_bool(&out_comm->is_enabled, true);
    sync_write_bool(&out_comm->is_app_handler_terminated, false);

    out_comm->app_comm_vaddr = params->sp_addrs.sp_comm_start;

    // Init <comm_sp>
    comm_sp->comm_os_ipis = 0;
    comm_sp->send_chan.comm_ctl_vaddr = out_comm->app_comm_vaddr;
    comm_sp->send_chan.comm_buf_vaddr = comm_sp->send_chan.comm_ctl_vaddr + PAGE_SIZE_4K;
    comm_sp->send_chan.counter_msgs = 0;
    comm_sp->send_chan.next_msg_id = 0;

    comm_sp->recv_chan.comm_ctl_vaddr = out_comm->app_comm_vaddr + IPC_CHAN_SIZE;
    comm_sp->recv_chan.comm_buf_vaddr = comm_sp->recv_chan.comm_ctl_vaddr + PAGE_SIZE_4K;
    comm_sp->recv_chan.counter_msgs = 0;
    comm_sp->recv_chan.next_msg_id = 0;

    pthread_create(&event_thread, NULL, _comm_app_sp_on_msg_arrival, out_comm);
}

static int _comm_app_sp_init(SP_USTRUCT *params, COMM_SP_APP *out_comm)
{
    if (params->sp_addrs.sp_comm_start == INVALID_VADDR)
    {
        // Initialize the App-SecProcess communication channel metadata only if it is not allocated. So the IPC lib can
        // still manage the channel though it is stale.
        APPCOMM_INFO("No App-SecProcess communication channels are created\n");

        out_comm->sp_id = params->sp_id;
        sync_write_bool(&out_comm->is_enabled, false);

        //// The handler will not run if the channel is not created. So the handler is terminated.
        sync_write_bool(&out_comm->is_app_handler_terminated, true);

        return 0;
    }

    // Fully initialize the App-SecProcess communication channel
    _do_comm_app_sp_init(params, out_comm);

    // On success
    return 0;
}

void _comm_app_sp_fini(COMM_SP_APP *comm)
{
    // Check: params must be valid
    if (!comm)
        return;

    // Stop the event handler thread
    sync_write_bool(&comm->is_enabled, false);

    // Busy loop until the handler in the application is terminated.
    while(!sync_read_bool(&comm->is_app_handler_terminated))
    {

    }
}

//! @brief Return a unique message ID of channel <chan>. Unique means the ID is never used and will be never re-used
//! during the lifetime of the channel.
static bool _ipc_get_msg_id(struct comm_ipc_channel *chan, uint32_t *out_new_id)
{
    if (!out_new_id)
        return false;
    if (chan->next_msg_id == INVALID_COMM_IPC_MESSAGE_ID)
        return false;

    *out_new_id = chan->next_msg_id;
    atomic_increase(&chan->next_msg_id);

    return true;
}

//! @brief Prepare the header of a <struct ipc_msg> message
//!
//! @return Return true on success
bool app_comm_sp_prepare_request(COMM_SP_APP *comm, struct ipc_msg *msg, bool req_response)
{
    bool status = true;
    struct comm_secapp *comm_sp = NULL;
    struct comm_ipc_channel *chan = NULL;

    // Check: params must be valid
    if (!comm || !msg)
        return false;
    comm_sp = &comm->comm;
    chan = &comm_sp->send_chan;

    status = _ipc_get_msg_id(chan, &msg->header.req_id);
    if (!status)
    {
        APPCOMM_ERR("Failed to generate message ID for the request message!\n");
        return false;
    }

    msg->header.direction = DIR_APP_TO_SECAPP;
    msg->header.handled = MSG_NOT_HANDLED;

    if (req_response)
        msg->header.req_response = true;
    else
        msg->header.req_response = false;

    return true;
}

struct ipc_msg *app_comm_sp_create_msg(COMM_SP_APP *comm, bool req_response)
{
    bool status = true;
    struct ipc_msg *msg = NULL;

    msg = (struct ipc_msg *)malloc(sizeof(struct ipc_msg));
    if (!msg)
    {
        APPCOMM_ERR("Create communication message error: No memory!\n");
        return NULL;
    }

    // Clear the msg
    memset((void *)msg, 0, sizeof(struct ipc_msg));
    msg->header.handled = MSG_EMPTY;

    // Prepare the msg
    status = app_comm_sp_prepare_request(comm, msg, req_response);
    if (!status)
    {
        APPCOMM_ERR("Failed to generate message!\n");
        free(msg);
        return NULL;
    }

    return msg;
}

void app_comm_sp_free_msg(struct ipc_msg *req_msg, struct ipc_msg *resp_msg)
{
    if (req_msg)
        free(req_msg);

    if (req_msg == resp_msg)
        return; // prevent double free
    else if (resp_msg)
        free(resp_msg);
}

//! @brief Send a message to the SecApp
int app_comm_sp_send_msg_nowait(COMM_SP_APP *comm, struct ipc_msg *msg, size_t data_size)
{
    struct comm_secapp *comm_sp = NULL;

    // Check: params must be valid
    if (!comm || !msg)
        return 1;
    comm_sp = &comm->comm;

    msg->header.handled = MSG_NOT_HANDLED;
    msg->header.data_size = data_size;

    while (!_try_rb_enqueue(comm_sp, msg))
        usleep(20);

    // On success
    return 0;
}

int comm_app_sp_create(SP_USTRUCT *params)
{
    int ret = 0;
    COMM_SP_APP *comm = NULL;

    // Check: params must be valid
    if (!params)
        return 1;

    comm = malloc(sizeof(COMM_SP_APP));
    if (!comm)
        return 1;

    ret = _comm_app_sp_init(params, comm);
    if (ret)
        goto comm_init_err;

    // Register the SecProcess-App communication
    ret = comm_app_sp_register(params->sp_id, comm);
    if (ret)
        goto comm_registration_err;

    // On success
    return 0;

comm_registration_err:
    _comm_app_sp_fini(comm);
comm_init_err:
    if (comm)
        free(comm);
    return 1;
}

int comm_app_sp_destroy(SP_USTRUCT *params)
{
    int ret = 0;
    COMM_SP_APP *comm = NULL;

    // Check: params must be valid
    if (!params)
        return 1;

    comm = comm_app_sp_get(params->sp_id);
    if (!comm)
        return 1;

    ret = comm_app_sp_unregister(params->sp_id);
    if (ret)
        return 1;

    _comm_app_sp_fini(comm);

    free(comm);

    // On success
    return 0;
}

/// @brief [Internal Function][For DesktopMgr] Mark the backend thread is used by DesktopMgr. 
/// @param  
void comm_app_sp_mark_dtm_backend_thread(void)
{
    _comm_app_sp_thread_dtm = true;
}





/********* Debug Functions *********/
__attribute__((unused)) static void comm_app_sp_dump(COMM_SP_APP *comm)
{
    APPCOMM_INFO("comm:%p\n", comm);

    if(comm)
    {
        APPCOMM_INFO("\t comm->sp_id:%u\n", comm->sp_id);

        APPCOMM_INFO("\t comm->app_comm_vaddr:%p\n", (void*)comm->app_comm_vaddr);

        APPCOMM_INFO("\t comm->is_enabled:%u\n", sync_read_bool(&comm->is_enabled));
        APPCOMM_INFO("\t comm->is_app_handler_terminated:%u\n", sync_read_bool(&comm->is_app_handler_terminated));
    }
}