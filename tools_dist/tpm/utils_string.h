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

#ifndef DEBUGGER_UTIL_H
#define DEBUGGER_UTIL_H

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

#ifndef __ASSEMBLY__
#include <os/os_headers.h>
#include <app_types.h>
#include <app_log.h>

// Function to convert a byte array to a hexadecimal string.
// The caller must ensure that the output_string has enough space for (2 * byte_array_size + 1) characters.
extern void byte_array_to_hex_string(const uint8_t *byte_array, size_t byte_array_size, char *output_string);

extern void PrintBin(const uint8_t* buffer, size_t length);

#endif // __ASSEMBLY__
#endif // DEBUGGER_UTIL_H