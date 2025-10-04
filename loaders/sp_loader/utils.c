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

void udelay(uint32_t usecs)
{
    volatile uint64_t i = 0;

    do
    {
        i++;
    } while (i < 6000 * (uint64_t)usecs); // We assume the CPU freq is 6GHz
}

void mdelay(uint32_t msecs)
{
    udelay(1000 * msecs);
}

int file_read_all(const char* filepath, void** buf, size_t* size)
{
    bool ret = false;
    uint8_t* data = NULL;

    // Check: params must be valid
    if(!filepath || !buf || !size)
        return -1;

    ret = file_get_size(filepath, size);
    if(!ret)
    {
        APP_ERR("Read file error: read size error!\n");
        return -1;
    }

    data = (uint8_t*)app_mm_alloc(*size);
    if(!data)
    {
        APP_ERR("Read file error: allocate buffer error!\n");
        return -1;
    }

    ret = file_read(filepath, data, 0, *size);
    if(!ret)
    {
        APP_ERR("Read file error: read file error!\n");
        return -1;
    }

    // On success
    *buf = data;
    return 0;
}

int file_read_check_size(const char *filepath, size_t file_size_limit, void* out_buffer, size_t* out_actual_size)
{
    int ret = 0;
    FILE *file = NULL;
    long fileSize;
    size_t read_size = 0;

    // Check: params must be valid
    if(!out_buffer)
        return -1;

    // Open the file in binary read mode
    file = fopen_normalized_path(filepath, "rb");
    if (file == NULL)
    {
        APP_ERR("Read file error: open file (%s) error!\n", filepath);
        return -1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);

    if(out_actual_size)
        *out_actual_size = fileSize;

    // Check the file size
    if(file_size_limit != 0)
    {
        if(fileSize > file_size_limit)
        {
            APP_ERR("Read file error: File size (%lu) is not expected as %lu bytes!\n", fileSize, file_size_limit);
            return -1;
        }
    }

    // Read the entire file into the out_buffer
    read_size = fread(out_buffer, 1, fileSize, file);
    if(read_size != fileSize)
    {
        APP_ERR("Read file error: Read error!\n");
        ret = -1;
        goto out;
    }

    // On success
    ret = 0;


out:
    // Close the file
    if(file)
        fclose(file);
    return ret;
}