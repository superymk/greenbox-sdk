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

#ifndef APP_EVENTS
#define APP_EVENTS

#include <app_stdlib.h>
#include <ipc/app_comm.h>
#include <ipc/app_comm_mgr.h>

#ifndef __ASSEMBLY__

#define APPCOMM_DEBUG(format, ...) \
	do{	\
		if((APP_COMM_LOG >= LOG_DEBUG) && (APP_COMM_LOG != LOG_NONE)) {  \
		printf("[App-COMM] "format, ## __VA_ARGS__); } }while(0)

#define APPCOMM_INFO(format, ...) \
	do{	\
		if((APP_COMM_LOG >= LOG_INFO) && (APP_COMM_LOG != LOG_NONE)) {  \
		printf("[App-COMM] "format, ## __VA_ARGS__); } }while(0)
#define APPCOMM_ERR(format, ...) \
	do{	\
		if((APP_COMM_LOG >= LOG_ERR) && (APP_COMM_LOG != LOG_NONE)) {  \
		printf("[App-COMM ERROR] "format, ## __VA_ARGS__); } }while(0)

// Forward declaration
struct ipc_msg;

struct comm_ipc_channel
{	
	// memory of the channel
	vaddr_t 			comm_ctl_vaddr; //!< The vaddr of the shared communication 
								//!< control block
	vaddr_t 			comm_buf_vaddr; //!< The vaddr of the shared communication 
								//!< buffer 

	// Information private to SecOS only
	lock_t 				chan_lock;
	
	// Perf statics 
	perf_counter_t 		counter_msgs; //!< The number of requests received

	// Request Info
	uint32_t			next_msg_id;
	
};

struct comm_secapp
{
	// Perf statics
	perf_counter_t comm_os_ipis; //!< The number of this communication channel invoked

	struct comm_ipc_channel send_chan; //!< Information of the SecOS-->OS channel
	struct comm_ipc_channel recv_chan; //!< Information of the OS-->SecOS channel
};

/// @brief A communication channel between the SecProcess and its loader (an untrusted application).
typedef struct _comm_sp_app
{
    spid_t sp_id;

    struct comm_secapp comm;
    vaddr_t app_comm_vaddr;

    bool is_enabled;
    bool is_app_handler_terminated; // True iff the handler in the untrusted application is terminated;
} COMM_SP_APP;

extern struct ipc_msg* app_comm_sp_create_msg(COMM_SP_APP *comm, bool req_response);
extern void app_comm_sp_free_msg(struct ipc_msg* req_msg, struct ipc_msg* resp_msg);

//! @brief Send a message to the SecProcess
extern int app_comm_sp_send_msg_nowait(COMM_SP_APP* comm, struct ipc_msg* msg, size_t data_size);

//! @brief Prepare the header of a <struct ipc_msg> message
extern bool app_comm_sp_prepare_request(COMM_SP_APP* comm, struct ipc_msg* msg, bool req_response);

/// @brief Create a SecProcess-App communication channel to the SecProcess <params->sp_id>.
extern int comm_app_sp_create(SP_USTRUCT *params);

/// @brief Destroy the SecProcess-App communication channel to the SecProcess <params->sp_id>.
extern int comm_app_sp_destroy(SP_USTRUCT *params);

#endif // __ASSEMBLY__

#endif // APP_EVENTS