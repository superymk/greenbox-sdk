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

#include <appddk.h>
#include <sl_headers.h>

#include <string.h>

// #define _POSIX_C_SOURCE 199309L   /* for nanosleep */
#include <time.h>
#include <unistd.h>
#include <errno.h>

static unsigned char hex_char_to_int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0; // Not a valid hexadecimal character
}

SL_STATUS str_to_byte_array(const char *input_string, size_t input_length, uint8_t *byte_array)
{
    size_t byte_array_size = input_length / 2;

    if (!input_string || !byte_array)
        return SL_EINVAL;

    for (size_t i = 0; i < byte_array_size; i++)
    {
        unsigned char high_nibble = hex_char_to_int(input_string[2 * i]);
        unsigned char low_nibble = hex_char_to_int(input_string[2 * i + 1]);
        byte_array[i] = (high_nibble << 4) | low_nibble;
    }

    return SL_SUCCESS;
}

SL_STATUS greenbox_get_root_path(char *out_path)
{
    if (!out_path)
        return SL_EINVAL;

    strncpy(out_path, apploader_get_exe_directory(), MAX_PATH_LEN);
    // sos_loader is in the <root folder>/sos/, so we need to modify <out_path> to go up one directory.
    strcat(out_path, "\\..\\");

    // On success
    return SL_SUCCESS;
}

int sleep_ms(long milliseconds)
{
    if (milliseconds <= 0)
        return 0;

    struct timespec req, rem;
    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000L;

    /* Loop in case nanosleep is interrupted by a signal (EINTR) */
    while (nanosleep(&req, &rem) == -1)
    {
        if (errno == EINTR)
        {
            req = rem; /* continue sleeping for the remaining time */
        }
        else
        {
            /* some other error—fall back to usleep if available */
            useconds_t usec = (milliseconds * 1000UL);
            if (usec == milliseconds * 1000UL)
            {
                usleep(usec);
                return 0;
            }
            return -1;
        }
    }
    return 0;
}