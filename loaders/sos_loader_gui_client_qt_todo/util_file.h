// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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