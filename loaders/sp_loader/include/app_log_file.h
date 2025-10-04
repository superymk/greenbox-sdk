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

#ifndef APP_LOG_FILE_H
#define APP_LOG_FILE_H

#ifndef __ASSEMBLY__

extern int app_log_file_init(const char *in_log_filepath);

extern void app_log_file_fini(void);

extern int app_log_file_vlog(const char *format, va_list args);

/// @brief Writes a formatted log message to a log file with a timestamp.
/// @param format The format string (printf-style).
/// @param Variable arguments corresponding to the format string.
/// @return SL_SUCCESS on success, SL_EINVAL on failure.
extern int app_log_file_log(const char *format, ...);

#endif // __ASSEMBLY__
#endif // APP_LOG_FILE_H