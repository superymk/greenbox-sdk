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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

static char log_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
static bool log_filepath_initialized = false;

int app_log_file_init(const char* in_log_filepath)
{
    // If <in_log_filepath> is NULL, then the caller does not want to use file logging. Disable file logging and return 
    // success
    if(!in_log_filepath)
    {
        log_filepath_initialized = false;
        return 0;
    }

    snprintf(log_filepath, sizeof(log_filepath), "%s", in_log_filepath);
    log_filepath_initialized = true;

    // Attempt to delete the log file
    if (remove(log_filepath) != 0)
    {
        if (errno == ENOENT)
        {
            // File does not exist; continue the init.
        }
        else
        {
            // An error occurred while trying to delete the file.
            return 1;
        }
    }

    // Create directories of the file if needed
    if (fs_create_directories(log_filepath) != 0)
    {
        // Create error
        return 1;
    }
    
    // On success
    return 0;
}

void app_log_file_fini(void)
{
    log_filepath_initialized = false;
}

int app_log_file_vlog(const char *format, va_list args)
{
    // Make sure the file logging subsystem is initialized.
    if (!log_filepath_initialized)
        return 1;

    // Open the log file in append mode.
    FILE *log_file = fopen_normalized_path(log_filepath, "a");
    if (log_file == NULL)
        return 1;

    // Get the current time.
    time_t now = time(NULL);
    if (now == ((time_t)-1))
    {
        fclose(log_file);
        return 1;
    }

    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        fclose(log_file);
        return 1;
    }

    // Write a timestamp to the log file.
    if (fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d ",
                t->tm_year + 1900,
                t->tm_mon + 1,
                t->tm_mday,
                t->tm_hour,
                t->tm_min,
                t->tm_sec) < 0)
    {
        fclose(log_file);
        return 1;
    }

    // Write the formatted log message using the va_list.
    if (vfprintf(log_file, format, args) < 0)
    {
        fclose(log_file);
        return 1;
    }

    // Close the log file.
    if (fclose(log_file) != 0)
        return 1;

    return 0;
}

int app_log_file_log(const char *format, ...)
{
    int ret = 0;
    va_list args;
    
    // Initialize the va_list.
    va_start(args, format);
    
    // Forward the arguments to the vlogging function.
    ret = app_log_file_vlog(format, args);
    
    // Clean up the va_list.
    va_end(args);
    
    return ret;
}