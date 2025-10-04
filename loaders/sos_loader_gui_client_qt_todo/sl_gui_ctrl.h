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