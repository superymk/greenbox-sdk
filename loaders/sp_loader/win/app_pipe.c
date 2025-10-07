// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <windows.h>

/*
#define SECOS_SERVER_PIPE_NAME	TEXT("\\\\.\\pipe\\SecOS_Server")

bool app_pipe_create(h_pipe_t* out_hpipe, size_t msg_size)
{	
	BOOL   fConnected = FALSE; 
	HANDLE hPipe = INVALID_HANDLE_VALUE; 
   
	hPipe = CreateNamedPipe( 
		SECOS_SERVER_PIPE_NAME,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		msg_size,                  // output buffer size 
		msg_size,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (hPipe == INVALID_HANDLE_VALUE) 
	{
		APP_ERR("CreateNamedPipe failed, GLE=%d.\n", GetLastError()); 
		return false;
	}

	if(out_hpipe)
		*out_hpipe = (h_pipe_t)hPipe;

	return true;
}

bool app_pipe_server_connect(h_pipe_t* hpipe)
{
	HANDLE _hPipe = (HANDLE)*hpipe;
	BOOL   fConnected = FALSE; 

	// Wait for the client to connect; if it succeeds, 
	// the function returns a nonzero value. If the function
	// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

	fConnected = ConnectNamedPipe(_hPipe, NULL) ? 
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

	if (fConnected) 
	{
		*hpipe = (h_pipe_t)_hPipe;
		return true;
	}

	return false;
}

bool app_pipe_client_connect(h_pipe_t* out_hpipe)
{
	HANDLE hPipe; 
	LPTSTR lpvMessage=TEXT("Default message from client."); 
	BOOL   fSuccess = FALSE; 
	DWORD  cbRead, cbToWrite, cbWritten, dwMode; 
	LPTSTR lpszPipename = SECOS_SERVER_PIPE_NAME; 

	// Try to open a named pipe; wait for it, if necessary. 
	while (1) 
	{ 
		hPipe = CreateFile( 
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 
		if (hPipe != INVALID_HANDLE_VALUE) 
			break; 

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 
		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
			APP_ERR("Could not open pipe. GLE=%d\n", GetLastError() ); 
			return false;
		}

		// All pipe instances are busy, so wait for 5 seconds. 
		if ( ! WaitNamedPipe(lpszPipename, 5000)) 
		{ 
			APP_ERR("Could not open pipe: 5 second wait timed out."); 
			return false;
		} 
	} 

	// The pipe connected; change to message-read mode. 
	dwMode = PIPE_READMODE_MESSAGE; 
	fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess) 
	{
		APP_ERR("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError() ); 
		return false;
	}
	
	if(out_hpipe)
		*out_hpipe = (h_pipe_t)hPipe;
	return true;
}

bool app_pipe_write(h_pipe_t in_hPipe, void* buf, size_t size)
{
	HANDLE hPipe = (HANDLE)in_hPipe; 
	bool   fSuccess = FALSE;
	
	fSuccess = WriteFile( 
		  hPipe,				  // pipe handle 
		  buf,			  // message 
		  size,			  // message length 
		  NULL,			  // bytes written 
		  NULL);				  // not overlapped 
	
	if ( !fSuccess) 
	{
		APP_ERR("WriteFile to pipe failed. GLE=%d\n", GetLastError() ); 
		return false;
	}

	return true;
}

bool app_pipe_read(h_pipe_t in_hPipe, void* buf, size_t size)
{
	HANDLE hPipe = (HANDLE)in_hPipe; 
	bool   fSuccess = FALSE;
	
	fSuccess = ReadFile( 
		  hPipe,				  // pipe handle 
		  buf,			  // message 
		  size,			  // message length 
		  NULL,			  // bytes written 
		  NULL);				  // not overlapped 
	
	if ( !fSuccess) 
	{
		APP_ERR("ReadFile to pipe failed. GLE=%d\n", GetLastError() ); 
		return false;
	}

	return true;
}


void app_pipe_close(h_pipe_t in_hPipe)
{
	HANDLE hPipe = (HANDLE)in_hPipe; 
	
	CloseHandle(hPipe); 
}

*/