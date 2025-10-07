// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOSLOADER_GUI_HEADER_H
#define SOSLOADER_GUI_HEADER_H

#include <wx/msgdlg.h>

extern "C"
{
    #include "sl_gui_ctrl.h"

    /// @brief Reserved name of the trusted DesktopMgr SecProcess.
    /// [NOTE] DesktopMgr is a SecService, not a SecApp. 
    #define TRUSTED_SP_DesktopMgr_NAME          "SecService-DesktopMgr"  // Must be the same with the definition in secos/include/public/sos_sp_intfs.h
}

#include "sl_gui_rpc.h"

#endif // SOSLOADER_GUI_HEADER_H