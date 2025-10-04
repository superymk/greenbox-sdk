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

#ifndef COMM_SOS_HANDLER_H
#define COMM_SOS_HANDLER_H

// Must correspond to status definitions used in <_handle_secos_msg>
#define SOS_COMM_SUCCESS		 			0

#ifndef __ASSEMBLY__
#include <os_types.h>
#include <public/os_errno.h>

//! Handle a message sent from SecOS/SecProcess
extern OSSTATUS comm_sos_handle_msg(struct comm_os_msg* msg);

extern void comm_sos_handler_init(void);

extern void comm_sos_handler_fini(void);

//! \brief Intercept the response message corresponding to request with <req_id> 
extern ListNode* comm_sos_intercept_response(msg_id_t req_id, struct comm_os_msg* out_msg);

//! \brief Cancel a previous comm_sos_intercept_response() call
extern void comm_sos_intercept_response_cancel(ListNode* wait_msg_node);

#endif // __ASSEMBLY__
#endif // COMM_SOS_HANDLER_H
