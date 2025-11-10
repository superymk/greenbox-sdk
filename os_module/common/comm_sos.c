// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>
#include "comm_sos_handler.h"

#define SECURITY_CHECK(x)                                                                   \
    {                                                                                       \
        do                                                                                  \
        {                                                                                   \
            if (!(x))                                                                       \
            {                                                                               \
                printk("[%s:%d]: SECURITY POLICY VIOLATION: %s\n", __FILE__, __LINE__, #x); \
                goto error;                                                                 \
            }                                                                               \
        } while (0);                                                                        \
    }

static struct comm_os_channel os_send_chan;
static struct comm_os_channel os_recv_chan;

OSSTATUS comm_sos_init(vaddr_t os_send_ctl, vaddr_t os_send_buf, vaddr_t os_recv_ctl, vaddr_t os_recv_buf)
{
    // Check parameters
    if (os_send_ctl == INVALID_VADDR || os_send_buf == INVALID_VADDR || os_recv_ctl == INVALID_VADDR || os_recv_buf == INVALID_VADDR)
    {
        SACOMM_ERR("Init SecOS-OS communication channel in SecAgent error!\n");
        return E_INV_ARG;
    }

    // Initialize the SecOS-OS channel
    os_send_chan.counter_msgs = 0;
    os_send_chan.comm_ctl_vaddr = os_send_ctl;
    os_send_chan.comm_buf_vaddr = os_send_buf;

    os_recv_chan.counter_msgs = 0;
    os_recv_chan.comm_ctl_vaddr = os_recv_ctl;
    os_recv_chan.comm_buf_vaddr = os_recv_buf;

    // Init handler of the SecOS-OS channel
    comm_sos_handler_init();

    return OS_SUCCESS;
}

void comm_sos_fini(void)
{
    // Finalize handler of the SecOS-OS channel
    comm_sos_handler_fini();

    // Finalize the SecOS-OS channel
    os_send_chan.counter_msgs = 0;
    os_send_chan.comm_ctl_vaddr = INVALID_VADDR;
    os_send_chan.comm_buf_vaddr = INVALID_VADDR;
    os->spinlock_init(&os_send_chan.chan_lock);

    os_recv_chan.counter_msgs = 0;
    os_recv_chan.comm_ctl_vaddr = INVALID_VADDR;
    os_recv_chan.comm_buf_vaddr = INVALID_VADDR;
    os->spinlock_init(&os_recv_chan.chan_lock);
}

static uint32_t _comm_ctl_get_p_slot(volatile struct comm_os_ctl *comm_ctl)
{
    os->cache_flush_range((vaddr_t)&comm_ctl->p_slot, sizeof(comm_ctl->p_slot));

    return comm_ctl->p_slot;
}

static void _comm_ctl_set_p_slot(volatile struct comm_os_ctl *comm_ctl, uint32_t new_p_slot)
{
    comm_ctl->p_slot = new_p_slot;
    os->cache_flush_range((vaddr_t)&comm_ctl->p_slot, sizeof(comm_ctl->p_slot));
}

static uint32_t _comm_ctl_get_c_slot(volatile struct comm_os_ctl *comm_ctl)
{
    os->cache_flush_range((vaddr_t)&comm_ctl->c_slot, sizeof(comm_ctl->c_slot));

    return comm_ctl->c_slot;
}

static void _comm_ctl_set_c_slot(volatile struct comm_os_ctl *comm_ctl, uint32_t new_c_slot)
{
    comm_ctl->c_slot = new_c_slot;
    os->cache_flush_range((vaddr_t)&comm_ctl->c_slot, sizeof(comm_ctl->c_slot));
}

static uint32_t _rb_increment(uint32_t val)
{
    os->atomic_increase(&val);

    return val % COMM_OS_CHAN_NUM_MSG_SLOTS;
}

static OSSTATUS _copy_to_comm_buf(volatile struct comm_os_msg *comm_buf, uint32_t p_slot, struct comm_os_msg *in_msg, uint32_t msg_size)
{
    if (p_slot >= COMM_OS_CHAN_NUM_MSG_SLOTS)
        return E_INV_ARG;

    os->mm_memcpy((void *)&comm_buf[p_slot], in_msg, msg_size);
    os->cache_flush_range((vaddr_t)&comm_buf[p_slot], msg_size);
    return OS_SUCCESS;
}

static OSSTATUS _copy_from_comm_buf(volatile struct comm_os_msg *comm_buf, uint32_t c_slot, void *buf, uint32_t buf_size)
{
    if (c_slot >= COMM_OS_CHAN_NUM_MSG_SLOTS)
        return E_INV_ARG;

    os->cache_flush_range((vaddr_t)&comm_buf[c_slot], buf_size);
    os->mm_memcpy(buf, (void *)&comm_buf[c_slot], buf_size);
    return OS_SUCCESS;
}

static OSSTATUS _try_rb_enqueue(struct comm_os_msg *in_msg)
{
    OSSTATUS status = OS_SUCCESS;
    volatile struct comm_os_msg *comm_buf = NULL;
    volatile struct comm_os_ctl *comm_ctl = NULL;
    struct comm_os_channel *chan = &os_send_chan;
    uint32_t p_slot = 0;
    uint32_t c_slot = 0;
    uint32_t next_p_slot = 0;
    uint32_t msg_size = 0;

    comm_ctl = CHAN_OS_CTL(chan);
    comm_buf = CHAN_OS_BUF(chan);

    // Check validity of <comm_ctl> and <comm_buf>
    if ((vaddr_t)comm_ctl == INVALID_VADDR || (vaddr_t)comm_buf == INVALID_VADDR)
    {
        SACOMM_ERR("Invalid SecOS-OS communication channel in SecAgent!\n");
        return E_INV_ARG;
    }

    // Get current producer counter
    os->spinlock_lock(&chan->chan_lock);
    p_slot = _comm_ctl_get_p_slot(comm_ctl);
    next_p_slot = _rb_increment(p_slot);
    c_slot = _comm_ctl_get_c_slot(comm_ctl);

    if (next_p_slot != c_slot)
    {
        // Set msg_size
        msg_size = COMM_OS_MSG_HEADER_SIZE + in_msg->header.data_size;
        msg_size = min(msg_size, COMM_OS_MSG_SIZE);

        // Commit message
        status = _copy_to_comm_buf(comm_buf, p_slot, in_msg, msg_size); // [TODO][Ticket 120] We do not need to copy it here.
        if (SA_FAIL(status))
            goto out;

        // Increase the producer counter
        os->mm_fence();
        _comm_ctl_set_p_slot(comm_ctl, next_p_slot);

        // Update perf counters
        perf_counter_inc(&chan->counter_msgs);

        status = OS_SUCCESS;
    }
    else
        status = E_COMM_FULL;

out:
    os->spinlock_unlock(&chan->chan_lock);
    return status;
}

static OSSTATUS _try_rb_dequeue(struct comm_os_msg *out_msg, bool *out_got_message)
{
    OSSTATUS status = OS_SUCCESS;
    bool got_message = false;
    volatile struct comm_os_msg *comm_buf = NULL;
    volatile struct comm_os_ctl *comm_ctl = NULL;
    struct comm_os_channel *chan = &os_recv_chan;
    uint32_t p_slot = 0;
    uint32_t c_slot = 0;

    comm_ctl = CHAN_OS_CTL(chan);
    comm_buf = CHAN_OS_BUF(chan);

    // Check validity of <comm_ctl> and <comm_buf>
    if ((vaddr_t)comm_ctl == INVALID_VADDR || (vaddr_t)comm_buf == INVALID_VADDR)
    {
        SACOMM_ERR("Invalid SecOS-OS communication channel in SecAgent!\n");
        return E_INV_ARG;
    }

    // Get current consumer counter
    os->spinlock_lock(&chan->chan_lock);
    p_slot = _comm_ctl_get_p_slot(comm_ctl);
    c_slot = _comm_ctl_get_c_slot(comm_ctl);

    if (c_slot != p_slot)
    {
        struct comm_os_header msg_header;
        uint32_t msg_size = 0;

        // Step 1. Inspect message header
        memset(&msg_header, 0, sizeof(msg_header)); // [NOTE] Use memset to initialize <msg_header> instead of initializing
                                                    // it in the local variable, as gcc may allocate non 16-byte aligned space for <msg_header> and use
                                                    // <movaps> instruction.

        status = _copy_from_comm_buf(comm_buf, c_slot, &msg_header, COMM_OS_MSG_HEADER_SIZE);
        if (SA_FAIL(status))
            goto out;

        msg_size = COMM_OS_MSG_HEADER_SIZE + msg_header.data_size;
        msg_size = min(msg_size, COMM_OS_MSG_SIZE);

        // Step 2. Read the whole message (w/ the header)
        status = _copy_from_comm_buf(comm_buf, c_slot, out_msg, msg_size); // [TODO][Ticket 120] We do not need to copy it here.
        if (SA_FAIL(status))
            goto out;

        // Increase the consumer counter
        os->mm_fence();
        _comm_ctl_set_c_slot(comm_ctl, _rb_increment(c_slot));

        // Update perf counters
        perf_counter_inc(&chan->counter_msgs);

        SACOMM_DEBUG("<_try_rb_dequeue> c_slot:%u p_slot:%u\n", c_slot, _comm_ctl_get_p_slot(comm_ctl));
        SACOMM_DEBUG("<_try_rb_dequeue> handled:%u, ack_req_id:%u\n", out_msg->header.handled, out_msg->header.ack_req_id);

        got_message = true;
        status = OS_SUCCESS;
        goto out;
    }

    got_message = false;
    status = OS_SUCCESS;

out:
    os->spinlock_unlock(&chan->chan_lock);
    *out_got_message = got_message;
    return status;
}

/// Triggered on a message arrival via SecOS-SecBase/OS communication channel
OSSTATUS comm_sos_handle_queued_msgs(bool* out_has_reqs)
{
    OSSTATUS status = OS_SUCCESS;
    struct comm_os_msg *msg;
    bool msg_ready = false;

    if (out_has_reqs)
        *out_has_reqs = false;

    msg = (struct comm_os_msg *)os->mm_malloc_zeroed(sizeof(struct comm_os_msg));
    if (!msg)
        return E_NO_MEM;

    do
    {
        status = _try_rb_dequeue(msg, &msg_ready);
        if (SA_FAIL(status))
        {
            // SecOS-OS communication channel error!
            goto handle_msg_err;
        }

        if (msg_ready)
        {
            if (out_has_reqs)
                *out_has_reqs = true;

            status = comm_sos_handle_msg(msg);
            if (SA_FAIL(status))
            {
                goto handle_msg_err;
            }
        }

    } while (msg_ready);

    // Inform App if necessary
    // sa_app_notify();

    os->mm_free(msg);

    return OS_SUCCESS;

handle_msg_err:
    os->mm_free(msg);
    return E_INTERNAL_ERROR;
}

/// @brief Send a message to the SecOS
///
/// This function also triggers the SecOS to handle the message
static OSSTATUS _comm_sos_send_msg(struct comm_os_msg *msg)
{
    OSSTATUS status = OS_SUCCESS;
    SACOMM_DEBUG("<_comm_sos_send_msg> cmd:%d\n", msg->header.cmd);

    if (!msg)
        return E_INV_ARG;

    // Send the message
    do
    {
        status = _try_rb_enqueue(msg);
        if (status == OS_SUCCESS)
        {
            notify_secos();
            break;
        }
        else if (status == E_COMM_FULL)
        {
            notify_secos();
            os->timer_usleep(20);
            continue;
        }
        else if (status == E_INV_ARG)
        {
            // Enqueue message error!
            return E_COMM_ERR;
        }
    } while (true);

    // notify_secos();

    return OS_SUCCESS;
}

OSSTATUS comm_sos_send_msg_nowait(struct comm_os_msg *msg)
{
    return _comm_sos_send_msg(msg);
}

OSSTATUS comm_sos_send_msg_and_wait(struct comm_os_msg *request, struct comm_os_msg *response, uint32_t ms)
{
    const ulong_t count_per_millisecond = 100; // 100 wait loops per 1ms wait time.
    const ulong_t ping_interval_cnt = 200 * count_per_millisecond; // re-ping SecOS for every 200ms.

    OSSTATUS status = OS_SUCCESS;
    ListNode *wait_msg_node = NULL;
    volatile msg_id_t *response_id = NULL;
    ulong_t cnt = ms * count_per_millisecond;
    msg_id_t request_id = 0;

    if (!request || !response)
        return E_INV_ARG;

    request_id = request->header.req_id;
    response_id = &response->header.ack_req_id;

    // Init the request_id in <response> to be different with the input <request_id>
    *response_id = request_id - 1;

    wait_msg_node = comm_sos_intercept_response(request_id, response);

    // Send the message
    status = _comm_sos_send_msg(request);
    if (SA_FAIL(status))
    {
        DRV_ERR("Failed to send message (req_id:%u) before waiting it!\n", request_id);
        comm_sos_intercept_response_cancel(wait_msg_node);
        return E_COMM_ERR;
    }

    SACOMM_DEBUG("<comm_sos_wait_msg> Wait for request ID:%u\n", request_id);
    while (*response_id != request_id)
    {
        if (!(--cnt))
        {
            comm_sos_intercept_response_cancel(wait_msg_node);
            return E_TIMEOUT; // Time out
        }

        status = comm_sos_handle_queued_msgs(NULL);
        if (SA_FAIL(status))
        {
            DRV_ERR("Failed to process the response of the message (req_id:%u)! status:%lu\n", request_id, status);
            comm_sos_intercept_response_cancel(wait_msg_node);
            return E_COMM_ERR;
        }

        os->udelay(10);

        if (!(cnt % ping_interval_cnt))
        {
            // Re-ping SecOS after every 200ms to make sure SecOS receive the SecOS-OS IPI interrupt.
            notify_secos();
        }
    }

    // On success
    return OS_SUCCESS;
}

/// @brief Return a unique message ID of channel <chan>. Unique means the ID is never used and will be never re-used
/// during the lifetime of the channel.
static OSSTATUS _comm_sos_get_msg_id(struct comm_os_channel *chan, msg_id_t *out_new_id)
{
    if (!out_new_id)
        return E_INV_ARG;
    if (chan->next_msg_id == INVALID_COMM_OS_MESSAGE_ID)
        return E_INV_ARG;

    *out_new_id = chan->next_msg_id;
    os->atomic_increase(&chan->next_msg_id);

    return OS_SUCCESS;
}

/// @brief Prepare the header of a <struct comm_os_msg> message
OSSTATUS comm_sos_prepare_request(struct comm_os_msg *msg, bool req_response, uint32_t dest_id)
{
    OSSTATUS status = OS_SUCCESS;
    volatile struct comm_os_msg *comm_buf = NULL;
    volatile struct comm_os_ctl *comm_ctl = NULL;
    struct comm_os_channel *chan = &os_send_chan;

    comm_ctl = CHAN_OS_CTL(chan);
    comm_buf = CHAN_OS_BUF(chan);
    (void)comm_ctl;
    (void)comm_buf;

    status = _comm_sos_get_msg_id(chan, &msg->header.req_id);
    if (SA_FAIL(status))
    {
        SACOMM_ERR("Failed to generate message ID for the request message! status:%lu\n", status);
        return E_INTERNAL_ERROR;
    }

    msg->header.direction = DIR_TO_S;
    msg->header.handled = REQUEST_NOT_HANDLED;
    msg->header.dest_id = dest_id;

    if (req_response)
        msg->header.req_response = true;
    else
        msg->header.req_response = false;

    return OS_SUCCESS;
}
