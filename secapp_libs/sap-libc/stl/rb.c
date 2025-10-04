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

#include <palddk.h>
#include <string.h>

static uint32_t _rb_increment(struct rb_channel* channel, uint32_t val)
{
	atomic_increase(&val);

	return val % channel->num_msg_slots;
}

static void _rb_flush_cache(void* ptr, size_t size)
{
	bool status;

	status = cache_flush_range((vaddr_t)ptr, size);
	if(!status)
	{
		SAP_WARN("Incorrect Ringbuffer communication: Cache range flush is not supported!\n");
	}
}

static bool _copy_to_comm_buf(struct rb_channel* channel, char* comm_buf, uint32_t p_slot, void* msg, uint32_t msg_size, bool flush_cache)
{
    if(p_slot >= channel->num_msg_slots)
        return false;

    {
        void* p_ptr = &comm_buf[p_slot * msg_size];
        memcpy(p_ptr, msg, msg_size);
        if(flush_cache)
            _rb_flush_cache(p_ptr, msg_size);
    }

    return true;
}

static bool _copy_from_comm_buf(struct rb_channel* channel, char* comm_buf, uint32_t c_slot, void* buf, uint32_t msg_size)
{
    if(c_slot >= channel->num_msg_slots)
        return false;

    memcpy(buf, (void*)&comm_buf[c_slot * msg_size], msg_size);
    return true;
}

static bool _try_rb_enqueue(struct rb_channel* channel, void* msg, bool flush_cache)
{
	bool status = false;
	char* comm_buf = NULL;
	struct rb_ctl* comm_ctl = NULL;
	uint32_t p_slot = 0;
	uint32_t next_p_slot = 0;

	if(!channel)
		return false;
	comm_ctl = (struct rb_ctl*)channel->comm_ctl_vaddr;
	comm_buf = (char*)channel->comm_buf_vaddr;
	
	// Get current producer counter
	p_slot = comm_ctl->p_slot;
	next_p_slot = _rb_increment(channel, p_slot);

	if(next_p_slot != comm_ctl->c_slot)
	{
		// Commit message
        status = _copy_to_comm_buf(channel, comm_buf, p_slot, msg, channel->msg_size, flush_cache);
        if(!status)
            goto out;
		
		// Increase the producer counter
		mfence();
		comm_ctl->p_slot = next_p_slot;
		if(flush_cache)
			_rb_flush_cache(comm_ctl, sizeof(struct rb_ctl));
		
		// // Update perf counters
		// perf_counter_inc(&channel->counter_msgs);

		// COMM_DEBUG("<_try_rb_enqueue> c_slot:%u p_slot:%u in_msg->header.cmd:%u\n", comm_ctl->c_slot, comm_ctl->p_slot, msg->header.cmd);

		status = true;
		goto out;
	}

	status = false;

out:
	return status;
}

static bool _try_rb_dequeue(struct rb_channel* channel, void* out_msg, bool* out_is_empty, bool flush_cache)
{
	bool status = false;
	char* comm_buf = NULL;
	struct rb_ctl* comm_ctl = NULL;
	uint32_t c_slot = 0;

	if(!channel || !out_msg)
		return false;
	comm_ctl = (struct rb_ctl*)channel->comm_ctl_vaddr;
	comm_buf = (char*)channel->comm_buf_vaddr;
	
	// Get current consumer counter
	c_slot = comm_ctl->c_slot;

	// COMM_DEBUG("<_try_rb_dequeue> c_slot:%u p_slot:%u\n", c_slot, comm_ctl->p_slot);
	if(c_slot != comm_ctl->p_slot)
	{
		uint32_t msg_size = channel->msg_size;
		
		// Step 2. Read the whole message (w/ the header)
        status = _copy_from_comm_buf(channel, comm_buf, c_slot, out_msg, msg_size); // [TODO][Ticket 120] We do not need to copy it here.
        if(!status)
            goto out;

		// Increase the consumer counter
		mfence();
		comm_ctl->c_slot = _rb_increment(channel, c_slot);
		if(flush_cache)
			_rb_flush_cache(comm_ctl, sizeof(struct rb_ctl));

		// // Update perf counters
		// perf_counter_inc(&channel->counter_msgs);
		
		status = true;
		*out_is_empty = false;

		goto out;
	}

	status = false;
	*out_is_empty = true;

out:
	return status;
}




/********* Public Functions *********/
SAP_STATUS rb_init(struct rb_channel* channel, vaddr_t rb_ctl_vaddr, vaddr_t rb_buf_vaddr, size_t msg_size, uint32_t num_msg_slots)
{
    // Check: Params are valid
    if(!channel || (rb_ctl_vaddr == INVALID_VADDR) || (rb_buf_vaddr == INVALID_VADDR))
		return EINVAL;

	// Check: msg_size * num_msg_slots must be non-zero 
	if(!msg_size || !num_msg_slots)
		return EINVAL;

    channel->comm_ctl_vaddr = rb_ctl_vaddr;
    channel->comm_buf_vaddr = rb_buf_vaddr;
	channel->msg_size = msg_size;
	channel->num_msg_slots = num_msg_slots;
    // channel->counter_msgs = 0;
    // channel->next_msg_id = 0;

	return SUCCESS;
}

SAP_STATUS rb_fini(struct rb_channel* channel)
{
	if(!channel)
		return EINVAL;
	
	channel->comm_ctl_vaddr = INVALID_VADDR;
	channel->comm_buf_vaddr = INVALID_VADDR;
	channel->msg_size = 0;
	channel->num_msg_slots = 0;

	return SUCCESS;
}

SAP_STATUS rb_send_msg(struct rb_channel* channel, void* msg, bool flush_cache)
{
	bool status = false;

	if(!channel || !msg)
		return EINVAL;

	status = _try_rb_enqueue(channel, msg, flush_cache);
	if(!status)
		return EINTERNAL;

	return SUCCESS;
}

SAP_STATUS rb_poll_msg(struct rb_channel* channel, void* out_msg, bool* out_is_empty, bool flush_cache)
{
	bool status = false;

	if(!channel || !out_is_empty || !out_msg)
		return EINVAL;

	status = _try_rb_dequeue(channel, out_msg, out_is_empty, flush_cache);
	if(!status)
		return EINTERNAL;

	return SUCCESS;
}