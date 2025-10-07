// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <execinfo.h>

#include <setjmp.h>
#include <signal.h>
#include <string.h>

#include <sl_headers.h>

#include "../hash/hash.h"

extern void sl_on_exception(void);

uint64_t rdtscll(void)
{
	uint32_t low, high;

	mfence();
	// [NOTE] Do not use cpuid, otherwise it will trap to hypervisor unconditionally
    asm volatile("rdtsc" : "=a" (low), "=d" (high)::);

    return UINT32_TO_64(high, low);
}

void sp_loader_on_exception(void)
{
	sl_on_exception();
}





/*
static void handler(int signal, siginfo_t *si, void *arg)
{
	void *array[30];
  	size_t size;

    printf("Caught segfault at address %p\n", si->si_addr);
	// get void*'s for all entries on the stack
	size = backtrace(array, 30);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", signal);
	backtrace_symbols_fd(array, size, STDERR_FILENO);

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


#define SECOS_SERVER_PIPE_NAME	"/tmp/SecOS_Server"

static void* thread_pipe_listener(void* data)
{
	FILE *fp = NULL;
	sl_calls_handler handler = NULL;

	handler = (sl_calls_handler)data;

	while(sl_state_get() == SECOS_UNLOADED)
	{
		if(!fp)
			fp = fopen_normalized_path(SECOS_SERVER_PIPE_NAME, "r"); //blocked fopen

		// do nothing, just wait the state elevating to SECOS_RUNNING
		
		// Check if the SecOS runs
		while(sl_state_get() == SECOS_RUNNING)
		{
			struct sl_cmd_packet msg = {0};
			// SecOS is running --> SecOS loader can handle application requests 
			// to SecOS now
			if(fread(&msg, sizeof(struct sl_cmd_packet), 1, fp) == 1)
				handler(&msg);

			msleep(100);
		}

		msleep(1000);
	}

	if(fp)
		fclose(fp);

	return NULL;
}

bool pipe_create_listen(sl_calls_handler handler)
{
	pthread_t pipe_thread;

	SL_INFO("Initialize pipe for apps\n");
	
	// Create the FIFO if it does not exist
	umask(0);
	mknod(SECOS_SERVER_PIPE_NAME, S_IFIFO|0666, 0);

	pthread_create(&pipe_thread, NULL, thread_pipe_listener, handler);

	return true;
}
*/