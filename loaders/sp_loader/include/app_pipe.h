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

#ifndef APP_PIPE_H
#define APP_PIPE_H

#include <app_types.h>

#ifndef __ASSEMBLY__
typedef void* h_pipe_t;

extern bool app_pipe_create(h_pipe_t* out_hpipe, size_t msg_size);
extern bool app_pipe_server_connect(h_pipe_t* hpipe);
extern bool app_pipe_client_connect(h_pipe_t* out_hpipe);
extern bool app_pipe_write(h_pipe_t in_hPipe, void* buf, size_t size);
extern bool app_pipe_read(h_pipe_t in_hPipe, void* buf, size_t size);
extern void app_pipe_close(h_pipe_t in_hPipe);


#endif // __ASSEMBLY__
#endif // APP_PIPE_H

