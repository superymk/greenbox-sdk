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

#ifndef APP_SP_INTFS_H
#define APP_SP_INTFS_H

#ifndef __ASSEMBLY__

#define RFILES_LOCAL_HOME_DIR_FILEPATH_MAXLEN      (512)

/// @brief Configurations of trusted path devices
struct sap_window_conf
{
    uint32_t win_logical_width;
    uint32_t win_logical_height;
    float win_scale;
};

struct sap_fs_local_conf
{
    char home_dir[RFILES_LOCAL_HOME_DIR_FILEPATH_MAXLEN];
};

typedef struct 
{
    struct sap_window_conf window_info;
    struct sap_fs_local_conf fs_local_info;
} SAP_CONFIG;

#endif // __ASSEMBLY__
#endif // APP_SP_INTFS_H