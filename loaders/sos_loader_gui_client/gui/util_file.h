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

#pragma once

#include "sl_gui.h"
#include <app_types.h>
#include <sl_status.h>

extern "C"
{
    #include <hash.h>
}

#define SECOS_CONFIG_FILE_RELATIVE_FILENAMEPATH "sos/sos_conf.json"

#define SL_GUI_LOG_ROOT_RELATIVE_FILEPATH "./userdata/logs/SOSLoader_GUI.log"

class FileUtils
{
    public:
        static char* getCurrentExecutablePath(void);
        static SL_STATUS greenbox_get_root_path(char* out_path);
        static SL_STATUS readFile(const char* filepath, bool create_if_not_exist, uint8_t **data, size_t *len);
        static SL_STATUS writeFile(const char* filepath, uint8_t* file_buf, size_t file_size);
        static SL_STATUS file_hash_and_check(const char* filepath, const union sha_digest *file_hash);
        static bool isFileExist(const char* relative_filepath);
        static SL_STATUS log(const char *format, ...);
        static SL_STATUS log_init(void);
        static void log_fini(void);

        /********* OS Specific Functions - Windows *********/
#if defined(__WINDOWS__)
        static SL_STATUS win_is_filepath_equal(const char* absolute_filepath1, const char* absolute_filepath2, bool* out_result);
#endif // __WINDOWS__
};