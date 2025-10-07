// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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