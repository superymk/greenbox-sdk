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

#ifdef __WINDOWS__

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <wolftpm/tpm2.h>
#include <wolftpm/tpm2_wrap.h>

#include <log.h>

#include "cmds.h"
#include "app_os_if.h"
#include "utils_string.h"
#include "utils.h"
#include "hypercall.h"

const union sha_digest _file_win_tpm2_aik_convert_bat_sha256 = {
    .sha2_256_digest = {
        0x7e, 0x7e, 0xf6, 0x1c, 0x24, 0x2b, 0x54, 0x16,
        0x6f, 0x9d, 0x45, 0xa6, 0x25, 0xc0, 0x2e, 0xfc,
        0x9f, 0x3a, 0xa8, 0xd3, 0x32, 0xed, 0x37, 0xb2,
        0xaf, 0x00, 0x6c, 0xdb, 0x1f, 0xd4, 0x08, 0x1e
    }
};

int tpm2_aik_convert_format(void)
{
    int ret = 0;
    char tpm_userdata_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* exe_filename = "win_tpm2_aik_convert.bat";

    gb_get_tpmtools_userdata_absolute_path(tpm_userdata_filepath);

    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);

    ret = os_execute_program_wait_exit(false, exe_filepath, &_file_win_tpm2_aik_convert_bat_sha256, exe_filename, tpm_userdata_filepath, NULL);
    if(ret)
    {
        printf("TPM Tool: Convert AIK error! ret:%d\n", ret);
        return -1;
    }

    return 0;
}


#endif // __WINDOWS__