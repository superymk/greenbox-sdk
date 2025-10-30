// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

// The function <comm_sos_send_msg_and_wait> implements block waiting on SecOS-OS communication channel. It behaves as follows:
// (1) OS must first create a request message, and calls <comm_sos_intercept_response> to register the request in <wait_msg_cache>
// (2) OS sends the message
// (3) OS checks responses until time out.
// After registering the request in <wait_msg_cache>, any errors must unregister the request.
//
// [NOTE] Why register-then-send a message, but not the opposite order?
// If a message is sent first, then its response may arrive before the sender registers the interception of the response.
// This causes additional lists to maintain the responses prior to registering their interceptions. Correct maintaining 
// the lists is complex; e.g., inserting/removing additional copies of messages in both lists.

struct msg_wait_info
{
	msg_id_t req_id;
	struct comm_os_msg* resp_buf;
};

//! SecOS may wait for responses. This cache is used to buffer responses.
//! [NOTE] All the responses must be fit in one single message
static List* wait_msg_cache = NULL;

//! The lock is for manipulate both <wait_msg_cache>
DECL_ALIGN(4) static lock_t wait_cache_lock;

//! Get and remove the waiting message
static struct comm_os_msg* _handler_pop_waitmsg(msg_id_t req_id);
extern OSSTATUS comm_sos_process_resp_msg(struct comm_os_msg* resp_msg);

/*static void dump_msg(struct comm_os_msg* msg)
{
	int i = 0;
	for (i = 0; i < 32; i++)
	{
		uint32_t* p = (uint32_t*)msg;

		printk("0x%08X ", p[i]);

		if( (i%8) == 7)
			printk("\n");
	}
	printk("\n");	
}*/

static OSSTATUS _comm_sos_handle_msg_ack(struct comm_os_msg* msg)
{
    // Mark the message as handled
    msg->header.handled = REQUEST_HANDLED;
    msg->header.ack_req_id = msg->header.req_id;
    msg->header.data_size = 0;

    return comm_sos_send_msg_nowait(msg);
}

//! Handle a message sent from SecOS/SecProcess
OSSTATUS comm_sos_handle_msg(struct comm_os_msg* msg)
{
	OSSTATUS status = OS_SUCCESS;

	if(!msg)
		return E_INV_ARG;

	if(msg->header.handled != REQUEST_HANDLED)
	{
		// The received message is a new request
		// Check the CMD of the message (a valid request to non-secure code (OS/Apps)?)
		OS_SECURITY_CHECK((msg->header.cmd > NS_CMD_START) && (msg->header.cmd < NS_CMD_INVALID));

        if(msg->header.dest_id == DEST_OS)
        {
            // The message is for the SecAgent

            // if the message needs an ack (SecOS-OS channel), we need to send it.
            if(msg->header.req_response)
			{
                status = _comm_sos_handle_msg_ack(msg);
				if(SA_FAIL(status))
				{
					SACOMM_ERR("Failed to send response (request to untrusted OS) via the SecOS-OS channel! status:%lu!\n", status);
					return E_INTERNAL_ERROR;
				}
			}
        }
        else
        {
            // The message is for an untrusted app
            process_id app_pid = (process_id)msg->header.dest_id;
            struct comm_os_app_siginfo_data* in_siginfo_data = (struct comm_os_app_siginfo_data*)msg->req_data;

            // Enqueue the signal sent from SecOS/SecProcess
            status = app_notif_enqueue_info(app_pid, in_siginfo_data->sp_id, in_siginfo_data->info1, in_siginfo_data->info2);
            if(SA_FAIL(status))
            {
                SACOMM_ERR("Enqueue SecOS/SecProcess's signal to untrusted process (%llu) error!\n", (uint64_t)app_pid);
                return E_INTERNAL_ERROR;
            }

            // if the message needs an ack (SecOS-OS channel), we need to send it before signaling the untrusted process.
            // Otherwise, the process is not guaranteed to return the control flow back to the SecAgent immediately after
            // handling the signal.
            if(msg->header.req_response)
            {
                status = _comm_sos_handle_msg_ack(msg);
				if(SA_FAIL(status))
				{
					SACOMM_ERR("Failed to send response (request to untrusted application) via the SecOS-OS channel! status:%lu!\n", status);
					return E_INTERNAL_ERROR;
				}
			}

            // Notify the corresponding untrusted process
            os->os_notify_app(app_pid);
        }
	}
	else if (msg->header.handled == REQUEST_HANDLED)
	{
		// The received message is a response to a previous request
		status = comm_sos_process_resp_msg(msg);
		if(SA_FAIL(status))
		{
			SACOMM_ERR("Handle response message (acking:%u) error! status:%lu!\n", msg->header.ack_req_id, status);
            return E_INTERNAL_ERROR;
		}
	}

	return OS_SUCCESS;
}

void comm_sos_handler_init(void)
{
    os->spinlock_init(&wait_cache_lock);

	if(!wait_msg_cache)
		wait_msg_cache = list_create();
}

void comm_sos_handler_fini(void)
{
	if(wait_msg_cache)
	{
		list_destroy(wait_msg_cache);
		wait_msg_cache = NULL;
	}
}




/********* Functions - Wait and Receive responses *********/
//! Get and remove the waiting message
static struct comm_os_msg* _handler_pop_waitmsg(msg_id_t req_id)
{
	struct comm_os_msg* wait_msg = NULL;
	struct msg_wait_info* v = NULL;
	
	LIST_FOREACH(wait_msg_cache, first, next, cur) 
	{
		v = (struct msg_wait_info*)cur->value;
		
		if(v->req_id == req_id)
		{
			wait_msg = v->resp_buf;
			break;
		}
	}
	END_LIST_FOREACH(wait_msg_cache);

	if(wait_msg)
	{
		// Found the waiting message
		list_remove(wait_msg_cache, cur);
		os->mm_free(v);
	}

	return wait_msg;
}

OSSTATUS comm_sos_process_resp_msg(struct comm_os_msg* resp_msg)
{
	OSSTATUS status = OS_SUCCESS;

	// Check: <resp_msg> must not be NULL
	if(!resp_msg)
		return E_INV_ARG;

	os->spinlock_lock(&wait_cache_lock);

	// See if there is a message wait request in <wait_msg_cache>
	{
		// The received message is a response to a previous request
		struct comm_os_msg* out_msg = NULL;
		
		out_msg = _handler_pop_waitmsg(resp_msg->header.ack_req_id);

		if(out_msg)
		{
			os->mm_memcpy(out_msg, resp_msg, sizeof(struct comm_os_msg));
			os->cache_flush_range((vaddr_t)out_msg, sizeof(struct comm_os_msg));
		}
		else
        {
            SACOMM_INFO("Ignore received response message (acking:%u) !\n", resp_msg->header.ack_req_id);
        }
	}

	SACOMM_DEBUG("<comm_sos_process_resp_msg> Get SecOS's Ack of request ID:%u\n", resp_msg->header.ack_req_id);
	status = OS_SUCCESS;

// out:
	os->spinlock_unlock(&wait_cache_lock);
	return status;
}

//! \brief Intercept the response message corresponding to request with <req_id> 
ListNode* comm_sos_intercept_response(msg_id_t req_id, struct comm_os_msg* out_msg)
{
	struct msg_wait_info* info = NULL;
	ListNode* node = NULL;

	info = os->mm_malloc_zeroed(sizeof(struct msg_wait_info));
	if (!info)
	{
		SACOMM_ERR("Intercept response error: No memory to create intercept info!\n");
		return NULL;
	}

	info->req_id = req_id;
	info->resp_buf = out_msg;

	os->spinlock_lock(&wait_cache_lock);
	node = list_enqueue(wait_msg_cache, info);
	os->spinlock_unlock(&wait_cache_lock);

	return node;
}

void comm_sos_intercept_response_cancel(ListNode* wait_msg_node)
{
	struct msg_wait_info* info;

	os->spinlock_lock(&wait_cache_lock);
	info = list_remove(wait_msg_cache, wait_msg_node);

	if(!info)
		os->mm_free(info);
		
	os->spinlock_unlock(&wait_cache_lock);
}
