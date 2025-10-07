#ifndef LIBC_BACKEND
#define LIBC_BACKEND

#include <app_types.h>

#include <inttypes.h>
#include <stddef.h>
#include <libc_backend_newlib_mappings.h>

#ifndef __ASSEMBLY__

struct libc_msg
{
	uint32_t	cmd;
	uint8_t* 	req_data;
	uint8_t* 	resp_data;
};

/// @brief Clear the OS errno. Required before calling any libc-backend functions.
/// @param  
extern void libc_clear_errno(void);

/// @brief Handle SecProcesses' libc functions calls that are not handled in the S partition. 
/// [NOTE] The calller of this function must allocate <msg->resp_data>.
/// @param msg 
/// @param out_valid_msg Required. Return true if the message contains a valid request to the libc backend.
/// @return 
extern size_t handle_libc_function(struct libc_msg* msg, bool* out_valid_msg);

/********* Debug Functions *********/
extern void _libc_backend_dbg_print_cmd(uint32_t cmd);
extern void _libc_backend_dbg_print_mem_as_string(char* prefix, void* buf, size_t len);

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND

