// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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