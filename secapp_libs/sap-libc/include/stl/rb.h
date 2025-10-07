// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// one-direction ring buffer, thread unsafe

#ifndef SECAPP_STL_RINGBUFFER_DROPNEW_H
#define SECAPP_STL_RINGBUFFER_DROPNEW_H

#ifndef __ASSEMBLY__

#define INVALID_COMM_OS_MESSAGE_ID		((msg_id_t) -1)

struct rb_channel
{	
	// memory of the channel
	vaddr_t 			comm_ctl_vaddr; //!< The vaddr of the shared communication 
								//!< control block
	vaddr_t 			comm_buf_vaddr; //!< The vaddr of the shared communication 
								//!< buffer

	size_t				msg_size;
	uint32_t			num_msg_slots;

	// lock_t 				reader_lock;
	
	// // Perf statics 
	// perf_counter_t 		counter_msgs; //!< The number of requests received

	// // Request Info
	// msg_id_t			next_msg_id;
	
};

// The shared information of ring buffer between SecOS and OS
struct rb_ctl
{
	uint32_t 	p_slot; //!< producer slot: the next msg slot to produce
	uint32_t	c_slot;
};

/// @brief Initialize a one-direction ring-buffer communication channel. The channel is not thread safe
/// @param channel 
/// @param rb_ctl_vaddr 
/// @param rb_buf_vaddr 
/// @param msg_size 
/// @param num_msg_slots 
/// @return 
extern SAP_STATUS rb_init(struct rb_channel* channel, vaddr_t rb_ctl_vaddr, vaddr_t rb_buf_vaddr, size_t msg_size, uint32_t num_msg_slots);

/// @brief 
/// @param channel 
/// @return 
extern SAP_STATUS rb_fini(struct rb_channel* channel);

/// @brief 
/// @param channel 
/// @param msg 
/// @param flush_cache 
/// @return 
extern SAP_STATUS rb_send_msg(struct rb_channel* channel, void* msg, bool flush_cache);

/// @brief 
/// @param channel 
/// @param out_msg 
/// @param out_is_empty 
/// @param flush_cache 
/// @return 
extern SAP_STATUS rb_poll_msg(struct rb_channel* channel, void* out_msg, bool* out_is_empty, bool flush_cache);

#endif // __ASSEMBLY__
#endif // SECAPP_STL_RINGBUFFER_DROPNEW_H