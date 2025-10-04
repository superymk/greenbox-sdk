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

extern "C"
{
    #include "sl_gui_ctrl.h"
    #include "sl_gui_utils.h"
    #include <string.h>
    #include "../loader/include/loader_os_intfs.h"

    static ulong_t flock_fd = 0;
    
    static bool sl_frm_spmgr_status = false;

    int sl_frm_main_instance_lock_grab(void)
    {
        char flock_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
        const char* flock_filename = "sl_sos_gui.lock";

        strncpy(flock_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
        strcat(flock_filepath, flock_filename);

        return os_file_lock_create(flock_filepath, &flock_fd);
    }

    int sl_frm_main_instance_lock_release(void)
    {
        return os_file_lock_release(flock_fd);
    }

    void sl_frm_spmgr_status_set(bool loaded)
    {
        sl_frm_spmgr_status = loaded;
    }
    bool sl_frm_spmgr_status_get(void)
    {
        return sl_frm_spmgr_status;
    }
}