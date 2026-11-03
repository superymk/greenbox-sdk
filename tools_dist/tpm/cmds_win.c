// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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
        0xb0, 0x84, 0xf3, 0x2f, 0x9c, 0x25, 0x2a, 0xc4,
        0x3e, 0x53, 0xd6, 0x6f, 0xc2, 0x93, 0x43, 0x9f,
        0x3e, 0x21, 0xa9, 0x3c, 0xc4, 0x32, 0xdd, 0x91,
        0xe9, 0x17, 0xe6, 0x1f, 0x35, 0xfd, 0xbc, 0x5f
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