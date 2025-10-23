#ifndef _LIBC_BACKEND_NEWLIB_MAPPINGS_H
#define _LIBC_BACKEND_NEWLIB_MAPPINGS_H

#include <inttypes.h>
#include <stddef.h>

#ifndef __ASSEMBLY__

/// @brief Map OS-specific posix errno to newlib's errno.
/// libc_backend only needs to transmit errnos defined in newlib. SecProcesses would not know how to handle errnos  
/// undefined in newlib.
extern int os_to_newlib_errno(int os_errno);




/********* Socket constants translate *********/

/// @brief Map newlib's socket recv function's flag constants to OS-specific posix definitions
/// @param newlib_socket_constant 
/// @return
int newlib_to_os_socket_send_recv_flags(int newlib_socket_constant);

extern int newlib_to_os_socket_addr_family(int newlib_socket_constant);
extern int os_to_newlib_socket_addr_family(int os_af);

extern int newlib_to_os_socket_protocol(int newlib_socket_constant);

/// @brief Map newlib's socket type constants to OS-specific posix definitions
/// @param newlib_socket_constant 
/// @return 
extern int newlib_to_os_socket_type(int newlib_socket_constant);

/// @brief Map newlib's socket option flag constants to OS-specific posix definitions
/// @param newlib_socket_constant 
/// @return 
extern int newlib_to_os_socket_option_flags(int newlib_socket_constant);

/// @brief Map newlib's socket option level constants to OS-specific posix definitions
/// @param newlib_socket_constant 
/// @return 
extern int newlib_to_os_socket_option_level(int newlib_socket_constant);

extern int newlib_to_os_socket_shutdown_howto(int newlib_socket_constant);

#endif // __ASSEMBLY__
#endif // _LIBC_BACKEND_NEWLIB_MAPPINGS_H

