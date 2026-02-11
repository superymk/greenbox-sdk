// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

/// @brief Initialize SecOS-OS communication channel
/// @param os_send_ctl Send control block vaddr
/// @param os_send_buf Send buffer vaddr
/// @param os_recv_ctl Receive control block vaddr
/// @param os_recv_buf Receive buffer vaddr
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_init(vaddr_t os_send_ctl, vaddr_t os_send_buf, vaddr_t os_recv_ctl, vaddr_t os_recv_buf);

/// @brief Finalize SecOS-OS communication channel
extern void comm_sos_fini(void);

/// @brief Handle queued messages from SecOS
/// @param out_has_reqs [Optional] Set to true if received SecOS requests
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_handle_queued_msgs(bool* out_has_reqs);

typedef enum 
{
	MSG_TO_SECOS,
	MSG_TO_SECAPP
} COMM_REQ_DEST;

/// @brief Send a message to the SecOS without waiting for response
/// [NOTE] This function also triggers the SecOS to handle the message
/// @param msg Message to send
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_send_msg_nowait(struct comm_os_msg* msg);

/// @brief Send a message to the SecOS and wait for response
/// [NOTE] This function also triggers the SecOS to handle the message
/// @param request Request message
/// @param response Response message
/// @param ms Timeout in milliseconds
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_send_msg_and_wait(struct comm_os_msg* request, struct comm_os_msg* response, uint32_t ms);

/// @brief Prepare the header of a comm_os_msg message
/// @param msg Message to prepare
/// @param req_response Whether response is required
/// @param dest_id Destination ID
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_prepare_request(struct comm_os_msg* msg, bool req_response,  uint32_t dest_id);

#endif // __ASSEMBLY__
#endif // COMM_SOS_TO_SECAGENT_H