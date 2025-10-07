// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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