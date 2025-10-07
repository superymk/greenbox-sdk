// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOSLOADER_GUI_CTRL_H
#define SOSLOADER_GUI_CTRL_H

extern "C"
{
    /// @brief Grab the file system lock to ensure the singleton instance of the SOSLoader GUI.
    /// @param  
    /// @return 
    int sl_frm_main_instance_lock_grab(void);

    /// @brief Release the file system lock to ensure the singleton instance of the SOSLoader GUI.
    /// @param  
    /// @return 
    int sl_frm_main_instance_lock_release(void);

    /// @brief Set false if the single instance of SOSLoader_GUIFrm_SPMgr is going to be closed. Set true if it is going to
    /// be created.
    /// @param loaded 
    void sl_frm_spmgr_status_set(bool loaded);

    /// @brief Return the current status of SOSLoader_GUIFrm_SPMgr instance.
    bool sl_frm_spmgr_status_get(void);
}

#endif // SOSLOADER_GUI_CTRL_H