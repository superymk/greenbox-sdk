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

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include "utils_string.h"

// Helper function to convert a single byte to its hexadecimal string representation.
static void byte_to_hex_string(uint8_t byte, char *hex_string) {
    const char hex_digits[] = "0123456789ABCDEF";
    hex_string[0] = hex_digits[(byte >> 4) & 0x0F];
    hex_string[1] = hex_digits[byte & 0x0F];
}

void byte_array_to_hex_string(const uint8_t *byte_array, size_t byte_array_size, char *output_string)
{
    for (size_t i = 0; i < byte_array_size; i++) {
        byte_to_hex_string(byte_array[i], &output_string[2 * i]);
    }
    output_string[2 * byte_array_size] = '\0'; // Null-terminate the string
}

#define LINE_LEN 16
void PrintBin(const uint8_t* buffer, size_t length)
{
    uint32_t i, sz;

    if (!buffer) {
        printf("\tNULL\n");
        return;
    }

    while (length > 0) {
        sz = length;
        if (sz > LINE_LEN)
            sz = LINE_LEN;

        printf("\t");
        for (i = 0; i < LINE_LEN; i++) {
            if (i < length)
                printf("%02x ", buffer[i]);
            else
                printf("   ");
        }
        printf("| ");
        for (i = 0; i < sz; i++) {
            if (buffer[i] > 31 && buffer[i] < 127)
                printf("%c", buffer[i]);
            else
                printf(".");
        }
        printf("\r\n");

        buffer += sz;
        length -= sz;
    }
}