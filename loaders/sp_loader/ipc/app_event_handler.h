// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_EVENT_HANDLER_H
#define APP_EVENT_HANDLER_H
#include <ipc/app_comm.h>

#ifndef __ASSEMBLY__

/// @brief Handle SecProcesses' requests sent to untrusted SecProcess loaders in the NS domain.
/// [NOTE] The calller of this function must allocate <req_msg>.
/// @param comm 
/// @param req_msg 
/// @param out_valid_msg Required. Return true if the message contains a valid request to the SecProcess loader.
/// @return 
extern bool ipc_handle_msg(COMM_SP_APP* comm, struct ipc_msg* req_msg, bool* out_valid_msg);

#endif // __ASSEMBLY__
#endif // APP_EVENT_HANDLER_H