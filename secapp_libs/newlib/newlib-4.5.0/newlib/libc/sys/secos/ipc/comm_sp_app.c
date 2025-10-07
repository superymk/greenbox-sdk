// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sys/ipc/comm.h>

#ifdef __i386__
#include <sys/machine/i386/atomic.h>
#elif defined __amd64__
#include <sys/machine/amd64/atomic.h>
#endif

#include <sys/lock.h>
#include <stddef.h>
#include <string.h>
#include <sap_headers_libc.h>

#include "../gloss/_libc_ops.h"
#include "../gloss/_i386_defs.h"
#include "comm_ops.h"

// The function <comm_send_msg_and_wait> implements block waiting on IPI communication channel. It behaves as follows:
// (1) sender must first create a request message, and calls <_comm_ipc_intercept_response> to register the request in <wait_msg_cache>
// (2) sender sends the message
// (3) sender checks responses until time out.
// After registering the request in <wait_msg_cache>, any errors must unregister the request.
//
// [NOTE] Why register-then-send a message, but not the opposite order?
// If a message is sent first, then its response may arrive before the sender registers the interception of the response.
// This causes additional lists to maintain the responses prior to registering their interceptions. Correct maintaining 
// the lists is complex; e.g., inserting/removing additional copies of messages in both lists.

static struct comm_secapp comm;
static List* wait_msg_cache = NULL;

//! The lock is for manipulate both <wait_msg_cache>
__LOCK_INIT(static, wait_cache_lock);

//! Get and remove the waiting message
static struct msg_wait_info* _handler_pop_waitmsg(uint32_t req_id);
extern bool _comm_ipc_process_resp_msg(struct ipc_msg* resp_msg);
//! \brief Intercept the response message corresponding to request with <req_id> 
static ListNode* _comm_ipc_intercept_response(uint32_t req_id, struct ipc_msg* resp_buf);
static void _comm_ipc_intercept_response_cancel(ListNode* wait_msg_node);
static void _comm_handle_queued_msgs(void);

#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

struct msg_wait_info
{
	uint32_t req_id;
	// bool handled;
	struct ipc_msg* resp_buf;
};

static uint32_t _comm_ctl_get_p_slot(volatile struct ipc_ctl* comm_ctl)
{
	(void)sos_sys_mm_flush_cache_range((vaddr_t)&comm_ctl->p_slot, sizeof(comm_ctl->p_slot));

	return comm_ctl->p_slot;
}

static void _comm_ctl_set_p_slot(volatile struct ipc_ctl* comm_ctl, uint32_t new_p_slot)
{
	comm_ctl->p_slot = new_p_slot;
	(void)sos_sys_mm_flush_cache_range((vaddr_t)&comm_ctl->p_slot, sizeof(comm_ctl->p_slot));
}

static uint32_t _comm_ctl_get_c_slot(volatile struct ipc_ctl* comm_ctl)
{
	(void)sos_sys_mm_flush_cache_range((vaddr_t)&comm_ctl->c_slot, sizeof(comm_ctl->c_slot));

	return comm_ctl->c_slot;
}

static void _comm_ctl_set_c_slot(volatile struct ipc_ctl* comm_ctl, uint32_t new_c_slot)
{
	comm_ctl->c_slot = new_c_slot;
	(void)sos_sys_mm_flush_cache_range((vaddr_t)&comm_ctl->c_slot, sizeof(comm_ctl->c_slot));
}

static uint32_t _rb_increment(uint32_t val)
{
	atomic_increase(&val);

	return val % IPC_CHAN_NUM_MSG_SLOTS;
}

static bool _copy_to_comm_buf(volatile struct ipc_msg* comm_buf, uint32_t p_slot, struct ipc_msg* in_msg, uint32_t msg_size)
{
    if(p_slot >= IPC_CHAN_NUM_MSG_SLOTS)
        return false;

    memcpy((void*)&comm_buf[p_slot], in_msg, msg_size);
	(void)sos_sys_mm_flush_cache_range((vaddr_t)&comm_buf[p_slot], msg_size);
    return true;
}

static bool _copy_from_comm_buf(volatile struct ipc_msg* comm_buf, uint32_t c_slot, void* buf, uint32_t buf_size)
{
    if(c_slot >= IPC_CHAN_NUM_MSG_SLOTS)
        return false;

	(void)sos_sys_mm_flush_cache_range((vaddr_t)&comm_buf[c_slot], buf_size);
    memcpy(buf, (void*)&comm_buf[c_slot], buf_size);
    return true;
}

static bool _try_rb_enqueue(struct ipc_msg* in_msg)
{
	bool status = false;
	volatile struct ipc_msg* comm_buf = NULL;
	volatile struct ipc_ctl* comm_ctl = NULL;
	struct comm_ipc_channel* chan = &comm.send_chan;
	uint32_t p_slot = 0;
	uint32_t c_slot = 0;
	uint32_t next_p_slot = 0;
	uint32_t msg_size = 0;

	comm_ctl = CHAN_SAP_CTL(chan);
	comm_buf = CHAN_SAP_BUF(chan);

	// Get current producer counter
	__lock_acquire(chan->chan_lock);
	p_slot = _comm_ctl_get_p_slot(comm_ctl);
	next_p_slot = _rb_increment(p_slot);
	c_slot = _comm_ctl_get_c_slot(comm_ctl);

	if(next_p_slot != c_slot)
	{
		// Set msg_size
		msg_size = IPC_MSG_HEADER_SIZE + in_msg->header.data_size;
		msg_size = min(msg_size, IPC_MSG_SIZE);
		
		// Commit message
        status = _copy_to_comm_buf(comm_buf, p_slot, in_msg, msg_size);  // [TODO][Ticket 120] We do not need to copy it here.
		if(!status)
            goto out;

		// Increase the producer counter
		mfence();
		_comm_ctl_set_p_slot(comm_ctl, next_p_slot);

		// Update perf counters
		atomic_increase(&chan->counter_msgs);
		
		status = true;
		goto out;
	}

	status = false;

out:
	__lock_release(chan->chan_lock);
	return status;
}

static bool _try_rb_dequeue(struct ipc_msg* out_msg)
{
	bool status = false;
	volatile struct ipc_msg* comm_buf = NULL;
	volatile struct ipc_ctl* comm_ctl = NULL;
	struct comm_ipc_channel* chan = &comm.recv_chan;
	uint32_t p_slot = 0;
	uint32_t c_slot = 0;

	comm_ctl = CHAN_SAP_CTL(chan);
	comm_buf = CHAN_SAP_BUF(chan);
	
	// Get current consumer counter
	__lock_acquire(chan->chan_lock);
	p_slot = _comm_ctl_get_p_slot(comm_ctl);
	c_slot = _comm_ctl_get_c_slot(comm_ctl);

	if(c_slot != p_slot)
	{
		struct ipc_header msg_header;
		uint32_t msg_size = 0;
		
		// Step 1. Inspect message header
		memset(&msg_header, 0, sizeof(msg_header)); // [NOTE] Use memset to initialize <msg_header> instead of initializing
						// it in the local variable, as gcc may allocate non 16-byte aligned space for <msg_header> and use
						// <movaps> instruction.
        status = _copy_from_comm_buf(comm_buf, c_slot, &msg_header, IPC_MSG_HEADER_SIZE);
		if(!status)
            goto out;

		msg_size = IPC_MSG_HEADER_SIZE + msg_header.data_size;
		msg_size = min(msg_size, IPC_MSG_SIZE); // [TODO] need to implement message split later
		
		// Step 2. Read the whole message (w/ the header)
        status = _copy_from_comm_buf(comm_buf, c_slot, out_msg, msg_size); // [TODO][Ticket 120] We do not need to copy it here.
		if(!status)
            goto out;

		// Increase the consumer counter
		mfence();
		_comm_ctl_set_c_slot(comm_ctl, _rb_increment(c_slot));

		// Update perf counters
		atomic_increase(&chan->counter_msgs);
		
		status = true;
		goto out;
	}

	status = false;

out:
	__lock_release(chan->chan_lock);
	return status;
}

static void _ipc_handle_msg(struct ipc_msg* msg)
{
	bool status = true;

	if(!msg)
		return;

	//COMM_DEBUG("<comm_os_handle_msg> handled:%d direction:%d cmd:%d\n", 
	//	msg->header.handled, msg->header.direction, msg->header.cmd);
	if(msg->header.cmd_type == RESPONSE)
	{
		// The received message is a response to a previous request
		status = _comm_ipc_process_resp_msg(msg);
		if(!status)
		{
			printf("Handle response message (acking:%u) error!\n", msg->header.ack_req_id);
            return;
		}
	}
	else
	{
		status = sp_handle_app_request(msg);
		if(!status)
		{
			printf("Handle App's request message (id:%u) error!\n", msg->header.req_id);
            return;
		}
	}
}




//! @brief Return a unique message ID of channel <chan>. Unique means the ID is never used and will be never re-used 
//! during the lifetime of the channel.
static bool _comm_get_msg_id(struct comm_ipc_channel* chan, uint32_t* out_new_id)
{
	if(!out_new_id)
		return false;
	if(chan->next_msg_id == INVALID_COMM_IPC_MESSAGE_ID)
		return false;

	*out_new_id = chan->next_msg_id;
	atomic_increase(&chan->next_msg_id);

	return true;
}

//! @brief Prepare the header of a <struct ipc_msg> message
static bool _comm_prepare_request(struct ipc_msg* msg, bool req_response)
{
	bool status = true;
	struct comm_ipc_channel* chan = &comm.send_chan;

	status = _comm_get_msg_id(chan, &msg->header.req_id);
	if(!status)
	{
		printf("Failed to generate message ID for the request message!\n");
		return false;
	}

	msg->header.direction = DIR_SECAPP_TO_APP;
	msg->header.handled = MSG_NOT_HANDLED;

	if(req_response)
		msg->header.req_response = true;
	else
		msg->header.req_response = false;

	return true;
}

/// @brief Create a message to be sent to the current SecProcess's loader red application.
/// @param req_response True if the message expects a response from the red application. Otherwise false.
/// @return If succeed, return the created new message in heap. Otherwise, return NULL.
struct ipc_msg* sp_comm_app_create_msg(bool req_response)
{
	bool status = true;
	struct ipc_msg* msg = NULL;
	uint32_t req_id = 0;

	msg = (struct ipc_msg*)smalloc(sizeof(struct ipc_msg));
	if(!msg)
	{
		printf("Create communication message error: No memory!\n");
		return NULL;
	}

	// Clear the msg
	memset((void*)msg, 0, sizeof(struct ipc_msg));
	msg->header.handled = MSG_EMPTY;

	// Prepare the msg
	status = _comm_prepare_request(msg, req_response);
	if(!status)
	{
		printf("Failed to generate message!\n");
		sfree(msg, sizeof(struct ipc_msg));
		return NULL;
	}

	return msg;
}

static bool _sp_comm_app_send_msg(struct ipc_msg* msg, size_t data_size)
{
	if(!msg)
		return false;

	msg->header.handled = MSG_NOT_HANDLED;
	msg->header.data_size = data_size;

	while(!_try_rb_enqueue(msg))
		usleep(20);

	return true;
}

bool sp_comm_app_send_msg_nowait(struct ipc_msg* msg, size_t data_size)
{
	return _sp_comm_app_send_msg(msg, data_size);
}

struct ipc_msg* sp_comm_app_send_msg_and_wait(struct ipc_msg* request, size_t data_size, uint32_t microsecs)
{
	bool status = false;
	ListNode* wait_msg_node = NULL;
	volatile uint32_t* response_id = NULL;
	unsigned long cnt = 0;
	uint32_t request_id = 0;
	struct ipc_msg* response = NULL;
	bool enable_wait = true;

	if(!request)
		return NULL;

	if(microsecs != COMM_WAIT_INFINITE)
	{
		enable_wait = true;
		cnt = microsecs * 100;
	}
	else
	{
		enable_wait = false;
	}

	// Allocate buffer for the response message
	response = (struct ipc_msg*)smalloc(sizeof(struct ipc_msg));
	if(!response)
	{
		printf("Send and wait message error: No memory to create wait information!\n");
		return NULL;
	}

	request_id = request->header.req_id;
	response_id = &response->header.ack_req_id;

	// Init the request_id in <response> to be different with the input <request_id>
	*response_id = request_id - 1;

	wait_msg_node = _comm_ipc_intercept_response(request_id, response);

	// Send the message
	status = _sp_comm_app_send_msg(request, data_size);
	if(!status)
	{
		printf("Failed to send message (req_id:%u) before waiting it!\n", request_id);
		_comm_ipc_intercept_response_cancel(wait_msg_node);
		goto err;
	}

	// Block wait the response
	while(*response_id != request_id)
	{
		if (enable_wait && (!(--cnt))) 
		{
			_comm_ipc_intercept_response_cancel(wait_msg_node);
			printf("Error on waiting response for message (req_id:%u)!\n", request_id);
			goto err; // Time out
		}

		_comm_handle_queued_msgs();

		usleep(10);
	}

	// On success
	return response;


err:
	if(response)
		sfree(response, sizeof(struct ipc_msg));
	return NULL;
}

// struct ipc_msg* sp_comm_app_wait_msg(uint32_t req_id)
// {
// 	struct ipc_msg* resp_msg = NULL;
	
// 	// Allocate buffer for the response message
// 	resp_msg = (struct ipc_msg*)smalloc(sizeof(struct ipc_msg));
// 	if(!resp_msg)
// 	{
// 		printf("Wait message error: No memory to create wait information!\n");
// 		return NULL;
// 	}

// 	// Block wait the response
// 	{
// 		volatile uint32_t* response_id = &resp_msg->header.ack_req_id;
// 		ListNode* wait_msg_node = NULL;

// 		// Init the req_id in <response> to be different with the input <req_id>
// 		*response_id = req_id - 1;

// 		wait_msg_node = _comm_ipc_intercept_response(req_id, resp_msg);

// 		while(*response_id != req_id)
// 		{
// 			_comm_handle_queued_msgs();
			
// 			usleep(10);
// 		}
// 	}

// 	return resp_msg;
// }

static void _comm_handle_queued_msgs(void)
{
	// Handle received App msgs
	volatile struct ipc_msg* msg = NULL;
	volatile bool msg_ready = false;

	msg = (struct ipc_msg*)smalloc(sizeof(struct ipc_msg));
	if(!msg)
	{
		printf("Wait message error: No memory!\n");
		return;
	}
		
	do
	{
		msg_ready = _try_rb_dequeue(msg);

		if(msg_ready)
			_ipc_handle_msg(msg);

		usleep(50);

	}while(msg_ready);

	sfree(msg, sizeof(struct ipc_msg));
}

/// @brief Force the current SecProcess to process all messages received from red applications.
/// @param  
void sp_comm_app_process_all_app_msgs(void)
{
	_comm_handle_queued_msgs();
}

void sp_comm_app_free_msg(struct ipc_msg* req_msg, struct ipc_msg* resp_msg)
{
	if(req_msg)
		sfree(req_msg, sizeof(struct ipc_msg));

	if(req_msg == resp_msg)
		return; // prevent double free
	else if(resp_msg)
		sfree(resp_msg, sizeof(struct ipc_msg));
}

/// @brief Initialize the communication channel to the current SecProcess's loader red application over the shared memory
/// <shared_mem_vaddr>.
/// @param shared_mem_vaddr Shared memory between the current SecProcess and its loader red application. 
void sp_comm_app_init(vaddr_t shared_mem_vaddr)
{
	// Clear <comm>'s buffer
	memset((void*)shared_mem_vaddr, 0, SAP_APP_COMM_SIZE);
	
	// Init <comm>
	comm.comm_os_ipis = 0;
	comm.recv_chan.comm_ctl_vaddr = shared_mem_vaddr;
	comm.recv_chan.comm_buf_vaddr = comm.recv_chan.comm_ctl_vaddr + PAGE_SIZE_4K;
	comm.recv_chan.counter_msgs = 0;
	comm.recv_chan.next_msg_id = 0;
	__lock_init(comm.recv_chan.chan_lock);

	comm.send_chan.comm_ctl_vaddr = shared_mem_vaddr + IPC_CHAN_SIZE;
	comm.send_chan.comm_buf_vaddr = comm.send_chan.comm_ctl_vaddr + PAGE_SIZE_4K;
	comm.send_chan.counter_msgs = 0;
	comm.send_chan.next_msg_id = 0;
	__lock_init(comm.send_chan.chan_lock);

	// Init <wait_msg_cache>
	if(!wait_msg_cache)
		wait_msg_cache = stl_list_create();

	{
		volatile struct ipc_ctl* recv_ctl = NULL;
		volatile struct ipc_ctl* send_ctl = NULL;

		recv_ctl = CHAN_SAP_CTL(&comm.recv_chan);
		send_ctl = CHAN_SAP_CTL(&comm.send_chan);
	}
}

void sp_comm_app_fini(void)
{
	if(wait_msg_cache)
	{
		stl_list_destroy(wait_msg_cache);
		wait_msg_cache = NULL;
	}
}

/// @brief Request the current SecProcess's loader red application allocate memory from the shared memory.
/// @param size the size of the memory to be allocated.
/// @return If succeed, return the offset in the shared memory. Otherwise, return INVALID_SMEM_OFFSET.
smem_offset_t sp_comm_app_alloc_sharedmem(size_t size)
{
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;
	smem_offset_t result = INVALID_SMEM_OFFSET;

	req_msg = sp_comm_app_create_msg(true);
	if(!req_msg)
		return INVALID_SMEM_OFFSET;

	req_msg->header.cmd_type = SHAREDMEM_OPS;
	req_msg->header.cmd = SHAREMEM_ALLOCATE;
	*(size_t*)req_msg->req_data = size;

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, sizeof(size), 1000);
	if(resp_msg)
	{
		result = *(smem_offset_t*)(&resp_msg->req_data);
	}
	else
	{
		result = INVALID_SMEM_OFFSET;
	}
	sp_comm_app_free_msg(req_msg, resp_msg);
	
	return result;
}

/// @brief Request the current SecProcess's loader red application to free the memory offset <offset> in the shared memory.
/// @param offset the offset of the shared memory to be freed.
/// @return If succeed, return true. Otherwise, return false.
bool sp_comm_app_free_sharedmem(smem_offset_t offset)
{
	bool result = false;
	struct ipc_msg* req_msg = NULL;
	struct ipc_msg* resp_msg = NULL;

	req_msg = sp_comm_app_create_msg(true);
	if(!req_msg)
		return false;

	req_msg->header.cmd_type = SHAREDMEM_OPS;
	req_msg->header.cmd = SHAREMEM_FREE;
	*(smem_offset_t*)req_msg->req_data = offset;

	// Send the request, and block waiting on the response
	resp_msg = sp_comm_app_send_msg_and_wait(req_msg, sizeof(offset), 1000);
	if(resp_msg)
	{
		result = true;
	}
	else
	{
		result = false;
	}
	sp_comm_app_free_msg(req_msg, resp_msg);

	return result;
}




/********* Functions - Wait and Receive responses *********/
//! Get and remove the waiting message
static struct msg_wait_info* _handler_pop_waitmsg(uint32_t req_id)
{
	bool found = false;
	struct msg_wait_info* v = NULL;
	
	LIST_FOREACH(wait_msg_cache, first, next, cur) 
	{
		v = (struct msg_wait_info*)cur->value;
		
		if(v->req_id == req_id)
		{
			found = true;
			break;
		}
    }
	END_LIST_FOREACH(wait_msg_cache);

	if(found)
	{
		// Found the waiting message, 
		stl_list_remove(wait_msg_cache, cur);
	}

	return v;
}

bool _comm_ipc_process_resp_msg(struct ipc_msg* resp_msg)
{
	bool status = true;

	// Check: <resp_msg> must not be NULL
	if(!resp_msg)
		return false;

	__lock_acquire(wait_cache_lock);

	// See if there is a message wait request in <wait_msg_cache>
	{
		// The received message is a response to a previous request
		volatile struct msg_wait_info* wait_info = NULL;

		wait_info = _handler_pop_waitmsg(resp_msg->header.ack_req_id);
		if(wait_info)
		{
			struct ipc_msg* out_msg = wait_info->resp_buf;
			memcpy(out_msg, resp_msg, sizeof(struct ipc_msg));
			// wait_info->handled = true;
			
			sfree(wait_info, sizeof(struct msg_wait_info));
		}
		else
		{
			printf("Ignore received response message (acking:%u) !\n", resp_msg->header.ack_req_id);
		}
	}

	// printf("<_comm_ipc_process_resp_msg> Get red App's Ack of request ID:%u\n", resp_msg->header.ack_req_id);
	status = true;

out:
	__lock_release(wait_cache_lock);
	return status;
}

//! \brief Intercept the response message corresponding to request with <req_id> 
static ListNode* _comm_ipc_intercept_response(uint32_t req_id, struct ipc_msg* resp_buf)
{
	struct msg_wait_info* info = NULL;
	ListNode* node = NULL;

	info = (struct msg_wait_info*)smalloc(sizeof(struct msg_wait_info));
	if(!info)
	{
		printf("Intercept response error: No memory to create intercept info!\n");
		return NULL;
	}

	info->req_id = req_id;
	// info->handled = false;
	info->resp_buf = resp_buf;

	__lock_acquire(wait_cache_lock);
	node = stl_list_enqueue(wait_msg_cache, info);
	__lock_release(wait_cache_lock);

	if(!node)
		sfree(info, sizeof(struct msg_wait_info));

	return node;
}

static void _comm_ipc_intercept_response_cancel(ListNode* wait_msg_node)
{
	struct msg_wait_info* info;

	__lock_acquire(wait_cache_lock);
	info = stl_list_remove(wait_msg_cache, wait_msg_node);

	if(!info)
		sfree(info, sizeof(struct msg_wait_info));
		
	__lock_release(wait_cache_lock);
}