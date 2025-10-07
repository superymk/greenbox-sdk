// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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
