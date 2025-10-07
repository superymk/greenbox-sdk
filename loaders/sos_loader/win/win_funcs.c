// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <pthread.h>
#include <app_pipe.h>

#include <windows.h>
#include <process.h>
#include <stdbool.h>
#include <stdarg.h>

#include <intrin.h>

// #include "../hash/hash.h"

// #include <sl_headers.h>

extern void sl_on_exception(void);

uint64_t rdtscll(void)
{
	return __rdtsc();
}

void sp_loader_on_exception(void)
{
	sl_on_exception();
}


/*
static void handler(int signal, siginfo_t *si, void *arg)
{
	// void *array[30];
  	// size_t size;

    printf("Caught segfault at address %p\n", si->si_addr);
	// get void*'s for all entries on the stack
	// size = backtrace(array, 30);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", signal);
	// backtrace_symbols_fd(array, size, STDERR_FILENO);

	// Call SecOS Loader's exception handler
	sl_on_exception();
}

void sl_install_exception_handler(void)
{
	struct sigaction sa;

	// Register sig fault handler
	memset(&sa, 0, sizeof(sigaction));
	sigemptyset(&sa.sa_mask);

	sa.sa_flags     = SA_NODEFER;
	sa.sa_sigaction = handler;

	sigaction(SIGSEGV, &sa, NULL); // ignore whether it works or not

	// Register Ctrl+C handler
	sigaction(SIGINT, &sa, NULL);
}


struct pipe_thread_param
{
	h_pipe_t hPipe;
	sl_calls_handler handler;
};

static void* thread_pipe_listener(void* data)
{
	sl_calls_handler handler = NULL;
	h_pipe_t hPipe; 
	struct pipe_thread_param* param = (struct pipe_thread_param*)data;
	
	hPipe = param->hPipe;
	handler = param->handler;

	while(sl_state_get() == SECOS_UNLOADED)
	{
		// do nothing, just wait the state elevating to SECOS_RUNNING
		
		// Check if the SecOS runs
		while(sl_state_get() == SECOS_RUNNING)
		{
			struct sl_cmd_packet msg = {0};
			bool   fSuccess = false; 

			app_pipe_server_connect(&hPipe);
			// SecOS is running --> SecOS loader can handle application requests 
			// to SecOS now
			fSuccess = app_pipe_read(hPipe, &msg, sizeof(struct sl_cmd_packet));
			if(fSuccess)
				handler(&msg);

			msleep(100);
		}

		msleep(1000);
	}

	app_pipe_close(hPipe);

	// free the param
	free(param);
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx
bool pipe_create_listen(sl_calls_handler handler)
{
	pthread_t pipe_thread;
	h_pipe_t hPipe;
	bool status = false;
	struct pipe_thread_param* param = NULL;
	
	SL_INFO("Initialize pipe for apps\n");
	
	status = app_pipe_create(&hPipe, sizeof(struct sl_cmd_packet));
	if(!status)
		return false;

	param = (struct pipe_thread_param*)malloc(sizeof(*param));
	param->hPipe = hPipe;
	param->handler = handler;
	
	pthread_create(&pipe_thread, NULL, thread_pipe_listener, param);

	return true;
}
*/