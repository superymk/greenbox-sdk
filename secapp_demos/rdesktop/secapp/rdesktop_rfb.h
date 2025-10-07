// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RDESKTOP_RFB_H
#define RDESKTOP_RFB_H

#include <sap_headers.h>
#include <palddk.h>

#include <rfb/rfbclient.h>

#ifndef __ASSEMBLY__

// /// @brief Perform RFB client initialization that should happen after connected to the RFB server; e.g., send client's 
// /// mouse position to the server.
// /// @param cl 
// /// @return 
// extern SAP_STATUS rdclient_rfb_init_on_connect(rfbClient* cl);

#endif // __ASSEMBLY__
#endif // RDESKTOP_RFB_H