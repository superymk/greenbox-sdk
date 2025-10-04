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

#ifndef SECOS_LOADER_UTILS_H
#define SECOS_LOADER_UTILS_H

#ifndef __ASSEMBLY__

/// @brief String to byte array
/// @param input_string 
/// @param input_length The length of input string exclude '\0' at the end
/// @param byte_array 
/// @return 
extern SL_STATUS str_to_byte_array(const char *input_string, size_t input_length, uint8_t* byte_array);

/// @brief Return the root folder of GreenBox.
/// @param out_path 
/// @return 
extern SL_STATUS greenbox_get_root_path(char* out_path);

/**
 * Sleep for the specified number of milliseconds.
 * Returns 0 on success, or -1 if an error (other than EINTR) occurs.
 */
extern int sleep_ms(long milliseconds);

#endif // __ASSEMBLY__
#endif // SECOS_LOADER_UTILS_H