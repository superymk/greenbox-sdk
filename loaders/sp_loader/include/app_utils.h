// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_UTILS_H
#define APP_UTILS_H

#ifndef __ASSEMBLY__

extern void mdelay(uint32_t msecs);
extern void udelay(uint32_t usecs);

extern int file_read_all(const char* filepath, void** buf, size_t* size);

/// @brief 
/// @param filepath 
/// @param file_size_limit If 0, then no limit. Otherwise, the function returns error if the actual file size larger 
/// than <file_size_limit>. 
/// @param out_buffer Required.
/// @param out_actual_size Optional. Updated with the actual file size.
/// @return 
extern int file_read_check_size(const char *filepath, size_t file_size_limit, void* out_buffer, size_t* out_actual_size);

#endif // __ASSEMBLY__
#endif // APP_UTILS_H