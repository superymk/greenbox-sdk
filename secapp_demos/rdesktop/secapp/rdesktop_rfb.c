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