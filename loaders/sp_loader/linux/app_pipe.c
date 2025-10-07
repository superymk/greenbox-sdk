// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

