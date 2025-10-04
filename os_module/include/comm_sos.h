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

#ifndef COMM_SOS_TO_SECAGENT_H
#define COMM_SOS_TO_SECAGENT_H

#ifndef __ASSEMBLY__
	
#define SACOMM_DEBUG(format, ...) \
	do{	\
		if((SACOMM_LOG >= LOG_DEBUG) && (SACOMM_LOG != LOG_NONE)) {  \
		PRINT("[CPU:%u][SA_COMM-Debug] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define SACOMM_INFO(format, ...) \
	do{	\
		if((SACOMM_LOG >= LOG_INFO) && (SACOMM_LOG != LOG_NONE)) {  \
		PRINT("[CPU:%u][SA_COMM] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define SACOMM_WARN(format, ...) \
	do{	\
		if((SACOMM_LOG >= LOG_WARNING) && (SACOMM_LOG != LOG_NONE)) {  \
		PRINT("[CPU:%u][SA_COMM-WARNING] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)

#define SACOMM_ERR(format, ...) \
	do{	\
		if((SACOMM_LOG >= LOG_ERR) && (SACOMM_LOG != LOG_NONE)) {  \
		PRINT_ERR("[CPU:%u][SA_COMM-ERR] "format, os->processor_id(), ## __VA_ARGS__); } }while(0)
	
#define INVALID_COMM_OS_MESSAGE_ID		((msg_id_t) -1)

struct comm_os_channel
{	
	// memory of the channel
	vaddr_t 		comm_ctl_vaddr; //!< The vaddr of the shared communication 
								//!< control block
	vaddr_t 		comm_buf_vaddr; //!< The vaddr of the shared communication 
								//!< buffer 

	// Information private to OS only
	lock_t 			chan_lock;
	
	// Perf statics 
	perf_counter_t 	counter_msgs; //!< The number of requests received

	// Request Info
	msg_id_t		next_msg_id;
};

extern OSSTATUS comm_sos_init(vaddr_t os_send_ctl, vaddr_t os_send_buf, vaddr_t os_recv_ctl, vaddr_t os_recv_buf);

extern void comm_sos_fini(void);

extern OSSTATUS comm_sos_handle_queued_msgs(void);

typedef enum 
{
	MSG_TO_SECOS,
	MSG_TO_SECAPP
} COMM_REQ_DEST;

/// @brief Send a message to the SecOS, but does not wait on response
///
/// This function also triggers the SecOS to handle the message
/// @return Return OS_SUCCESS on success
extern OSSTATUS comm_sos_send_msg_nowait(struct comm_os_msg* msg);

/// @brief Send a message to the SecOS and block wait on response for milliseconds <ms>.
///
/// This function also triggers the SecOS to handle the message
extern OSSTATUS comm_sos_send_msg_and_wait(struct comm_os_msg* request, struct comm_os_msg* response, uint32_t ms);

/// @brief Prepare the header of a <struct comm_os_msg> message
extern OSSTATUS comm_sos_prepare_request(struct comm_os_msg* msg, bool req_response,  uint32_t dest_id);

#endif // __ASSEMBLY__
#endif // COMM_SOS_TO_SECAGENT_H