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

#define SECOS_SERVER_PIPE_NAME	"/tmp/SecOS_Server"

bool app_pipe_client_connect(h_pipe_t* out_hpipe)
{
	FILE* fp = NULL;
	
	if((fp = fopen_normalized_path(SECOS_SERVER_PIPE_NAME, "w")) == NULL) 
	{
		APP_ERR("Connect SecOS Loader Error!\n");
		return false;
	}

	if(out_hpipe)
		*out_hpipe = (h_pipe_t)fp;
	
	return true;
}

bool app_pipe_write(h_pipe_t in_hPipe, void* buf, size_t size)
{
	FILE* fp = (FILE*)in_hPipe;

	fwrite(buf, size, 1, fp);

	return true;
}

void app_pipe_close(h_pipe_t in_hPipe)
{
	FILE* fp = (FILE*)in_hPipe;
	
	fclose(fp);
}

