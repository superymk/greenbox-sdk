// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef COMM_SOS_HANDLER_H
#define COMM_SOS_HANDLER_H

// Must correspond to status definitions used in <_handle_secos_msg>
#define SOS_COMM_SUCCESS		 			0

#ifndef __ASSEMBLY__
#include <os_types.h>
#include <public/os_errno.h>

/// @brief Handle a message sent from SecOS/SecProcess
/// @param msg Message to handle
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_handle_msg(struct comm_os_msg* msg);

/// @brief Initialize SecOS-OS communication handler
extern void comm_sos_handler_init(void);

/// @brief Finalize SecOS-OS communication handler
extern void comm_sos_handler_fini(void);

/// @brief Intercept the response message corresponding to request with <req_id>
/// @param req_id Request ID
/// @param out_msg Output message buffer
/// @return List node for the wait entry
extern ListNode* comm_sos_intercept_response(msg_id_t req_id, struct comm_os_msg* out_msg);

/// @brief Cancel a previous comm_sos_intercept_response() call
/// @param wait_msg_node Wait list node to cancel
extern void comm_sos_intercept_response_cancel(ListNode* wait_msg_node);

/// @brief Process a response message from SecOS
/// @param resp_msg Response message
/// @return OS_SUCCESS on success, error code on failure
extern OSSTATUS comm_sos_process_resp_msg(struct comm_os_msg* resp_msg);

#endif // __ASSEMBLY__
#endif // COMM_SOS_HANDLER_H
