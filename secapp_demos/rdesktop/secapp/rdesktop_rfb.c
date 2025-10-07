// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>

#include "rdesktop_rfb.h"
#include "rdesktop_hid.h"

// SAP_STATUS rdclient_rfb_init_on_connect(rfbClient* cl)
// {
//     uint16_t mouse_pos_x = 0, mouse_pos_y = 0;

//     // Check: params must be valid.
//     if(!cl)
//         return EINVAL;

//     // Send current mouse position to the RFB server.
//     libgui_mouse_get_pos(&mouse_pos_x, &mouse_pos_y);

//     rfb_send_mouse_pos(cl, (int)mouse_pos_x, (int)mouse_pos_y);

//     // On success
//     return SUCCESS;
// }