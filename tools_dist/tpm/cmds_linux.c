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

#ifdef __LINUX__

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cmds.h"
#include "app_os_if.h"
#include "utils_string.h"
#include "utils.h"
#include "hypercall.h"
#include "./hash/hash.h"

const union sha_digest _file_linux_tpm2_aik_convert_sh_sha256 = {
    .sha2_256_digest = {
        0x54, 0x50, 0x27, 0xf5, 0x59, 0xe5, 0xff, 0x90,
        0x5d, 0xa8, 0xd5, 0x20, 0x62, 0x60, 0x8d, 0xb4,
        0x53, 0xf3, 0xb4, 0x26, 0xff, 0x29, 0xd3, 0x8a,
        0x53, 0x98, 0x6b, 0xc6, 0x49, 0x86, 0x78, 0xe1
    }
};

 


int tpm2_aik_convert_format(void)
{
    int ret = 0;
    char tpm_userdata_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char exe_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    const char* exe_filename = "linux_tpm2_aik_convert.sh";

    gb_get_tpmtools_userdata_absolute_path(tpm_userdata_filepath);

    strncpy(exe_filepath, apploader_get_exe_directory(), MAX_PATH_LEN);
    strcat(exe_filepath, exe_filename);

    ret = os_execute_program_wait_exit(false, exe_filepath, &_file_linux_tpm2_aik_convert_sh_sha256, exe_filename, tpm_userdata_filepath, NULL);
    if(ret)
    {
        printf("TPM Tool: Convert AIK error! ret:%d\n", ret);
        return -1;
    }

    return 0;
}

#endif // __LINUX__