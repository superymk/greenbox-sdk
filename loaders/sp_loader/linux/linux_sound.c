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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <app_types.h>
#include <secloader_status.h>

__attribute__((unused)) static int get_muted(bool *muted)
{
    if (!muted)
        return 1;
    FILE *fp = popen("wpctl get-volume @DEFAULT_AUDIO_SINK@ 2>/dev/null", "r");
    if (!fp)
        return 1;
    char buf[256] = {0};
    if (!fgets(buf, sizeof buf, fp))
    {
        pclose(fp);
        return 1;
    }
    pclose(fp);
    *muted = strstr(buf, "MUTED") != NULL;
    return 0;
}

/********* Public Functions *********/
int _do_os_sound_mute(uint8_t *out_already_muted)
{
    // bool m = false;
    // if (!out_already_muted || get_muted(&m))
    //     return 1;
    // *out_already_muted = m;
    // if (m)
    //     return 0;
    // return system("wpctl set-mute @DEFAULT_AUDIO_SINK@ 1") == 0 ? 0 : 1;

    return 0;
}

int _do_os_sound_unmute(uint8_t already_muted)
{
    // if (already_muted)
    //     return 0;
    // return system("wpctl set-mute @DEFAULT_AUDIO_SINK@ 0") == 0 ? 0 : 1;

    return 0;
}