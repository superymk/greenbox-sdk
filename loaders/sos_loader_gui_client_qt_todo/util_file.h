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

#define SECOS_CONFIG_FILE_RELATIVE_FILENAMEPATH "sos/sos_conf.json"

class FileUtils
{
    public:
        static char* getCurrentExecutablePath(void);
        static SL_STATUS writeFile(const char* relative_filepath, uint8_t* file_buf, size_t file_size);
        static bool isFileExist(const char* relative_filepath);
};