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

#include <stdint.h>


/********* Errno translate *********/
#include <errno.h>

// Translate OS errnos into Newlib errnos
static uint32_t _os_to_newlib_errno[256] = 
{
    [EPERM] 1,               /* Not owner */
    [ENOENT] 2,              /* No such file or directory */
    [ESRCH] 3,               /* No such process */
    [EINTR] 4,               /* Interrupted system call */
    [EIO] 5,                 /* I/O error */
    [ENXIO] 6,               /* No such device or address */
    [E2BIG] 7,               /* Arg list too long */
    [ENOEXEC] 8,             /* Exec format error */
    [EBADF] 9,               /* Bad file number */
    [ECHILD] 10,             /* No children */
    [EAGAIN] 11,             /* No more processes */
    [ENOMEM] 12,             /* Not enough space */
    [EACCES] 13,             /* Permission denied */
    [EFAULT] 14,             /* Bad address */
    [ENOTBLK] 15,            /* Block device required */
    [EBUSY] 16,              /* Device or resource busy */
    [EEXIST] 17,             /* File exists */
    [EXDEV] 18,              /* Cross-device link */
    [ENODEV] 19,             /* No such device */
    [ENOTDIR] 20,            /* Not a directory */
    [EISDIR] 21,             /* Is a directory */
    [EINVAL] 22,             /* Invalid argument */
    [ENFILE] 23,             /* Too many open files in system */
    [EMFILE] 24,             /* File descriptor value too large */
    [ENOTTY] 25,             /* Not a character device */
    [ETXTBSY] 26,            /* Text file busy */
    [EFBIG] 27,              /* File too large */
    [ENOSPC] 28,             /* No space left on device */
    [ESPIPE] 29,             /* Illegal seek */
    [EROFS] 30,              /* Read-only file system */
    [EMLINK] 31,             /* Too many links */
    [EPIPE] 32,              /* Broken pipe */
    [EDOM] 33,               /* Mathematics argument out of domain of function */
    [ERANGE] 34,             /* Result too large */
    [ENOMSG] 35,             /* No message of desired type */
    [EIDRM] 36,              /* Identifier removed */
    [ECHRNG] 37,             /* Channel number out of range */
    [EL2NSYNC] 38,           /* Level 2 not synchronized */
    [EL3HLT] 39,             /* Level 3 halted */
    [EL3RST] 40,             /* Level 3 reset */
    [ELNRNG] 41,             /* Link number out of range */
    [EUNATCH] 42,            /* Protocol driver not attached */
    [ENOCSI] 43,             /* No CSI structure available */
    [EL2HLT] 44,             /* Level 2 halted */
    [EDEADLK] 45,            /* Deadlock */
    [ENOLCK] 46,             /* No lock */
    [EBADE] 50,              /* Invalid exchange */
    [EBADR] 51,              /* Invalid request descriptor */
    [EXFULL] 52,             /* Exchange full */
    [ENOANO] 53,             /* No anode */
    [EBADRQC] 54,            /* Invalid request code */
    [EBADSLT] 55,            /* Invalid slot */
    [EDEADLOCK] 56,          /* File locking deadlock error */
    [EBFONT] 57,             /* Bad font file format */
    [ENOSTR] 60,             /* Not a stream */
    [ENODATA] 61,            /* No data (for no delay io) */
    [ETIME] 62,              /* Stream ioctl timeout */
    [ENOSR] 63,              /* No stream resources */
    [ENONET] 64,             /* Machine is not on the network */
    [ENOPKG] 65,             /* Package not installed */
    [EREMOTE] 66,            /* The object is remote */
    [ENOLINK] 67,            /* Virtual circuit is gone */
        [EADV] 68,               /* Advertise error */
    [ESRMNT] 69,             /* Srmount error */
    [ECOMM] 70,              /* Communication error on send */
    [EPROTO] 71,             /* Protocol error */
    [EMULTIHOP] 74,          /* Multihop attempted */
    [EDOTDOT] 76,            /* Cross mount point (not really error) */
    [EBADMSG] 77,            /* Bad message */
    // [EFTYPE] 79,             /* Inappropriate file type or format */
    [ENOTUNIQ] 80,           /* Given log. name not unique */
    [EBADFD] 81,             /* f.d. invalid for this operation */
    [EREMCHG] 82,            /* Remote address changed */
    [ELIBACC] 83,            /* Can't access a needed shared lib */
    [ELIBBAD] 84,            /* Accessing a corrupted shared lib */
    [ELIBSCN] 85,            /* .lib section in a.out corrupted */
    [ELIBMAX] 86,            /* Attempting to link in too many shared libs */
    [ELIBEXEC] 87,           /* Attempting to exec a shared library */
    [ENOSYS] 88,             /* Function not implemented */
    // [ENMFILE] 89,            /* No more files */
    [ENOTEMPTY] 90,          /* Directory not empty */
    [ENAMETOOLONG] 91,       /* File or path name too long */
    [ELOOP] 92,              /* Too many symbolic links */
    [EOPNOTSUPP] 95,         /* Operation not supported on socket */
    [EPFNOSUPPORT] 96,       /* Protocol family not supported */
    [ECONNRESET] 104,        /* Connection reset by peer */
    [ENOBUFS] 105,           /* No buffer space available */
    [EAFNOSUPPORT] 106,      /* Address family not supported by protocol family */
    [EPROTOTYPE] 107,        /* Protocol wrong type for socket */
    [ENOTSOCK] 108,          /* Socket operation on non-socket */
    [ENOPROTOOPT] 109,       /* Protocol not available */
    [ESHUTDOWN] 110,         /* Can't send after socket shutdown */
    [ECONNREFUSED] 111,      /* Connection refused */
    [EADDRINUSE] 112,        /* Address already in use */
    [ECONNABORTED] 113,      /* Software caused connection abort */
    [ENETUNREACH] 114,       /* Network is unreachable */
    [ENETDOWN] 115,          /* Network interface is not configured */
    [ETIMEDOUT] 116,         /* Connection timed out */
    [EHOSTDOWN] 117,         /* Host is down */
    [EHOSTUNREACH] 118,      /* Host is unreachable */
    [EINPROGRESS] 119,       /* Connection already in progress */
    [EALREADY] 120,          /* Socket already connected */
    [EDESTADDRREQ] 121,      /* Destination address required */
    [EMSGSIZE] 122,          /* Message too long */
    [EPROTONOSUPPORT] 123,   /* Protocol not supported */
    [ESOCKTNOSUPPORT] 124,   /* Socket type not supported */
    [EADDRNOTAVAIL] 125,     /* Address not available */
    [ENETRESET] 126,         /* Connection aborted by network */
    [EISCONN] 127,           /* Socket is already connected */
    [ENOTCONN] 128,          /* Socket is not connected */
    [ETOOMANYREFS] 129,      /* Too many references */
    // [EPROCLIM] 130,          /* Process limit reached */
    [EUSERS] 131,            /* Too many users */
    [EDQUOT] 132,            /* Disk quota exceeded */
    [ESTALE] 133,            /* Stale file handle */
    [ENOTSUP] 134,           /* Not supported */
    [ENOMEDIUM] 135,         /* No medium (in tape drive) */
    // [ENOSHARE] 136,          /* No such host or network path */
    // [ECASECLASH] 137,        /* Filename exists with different case */
    [EILSEQ] 138,            /* Illegal byte sequence */
    [EOVERFLOW] 139,         /* Value too large for defined data type */
    [ECANCELED] 140,         /* Operation canceled */
    [ENOTRECOVERABLE] 141,   /* State     not recoverable */
    [EOWNERDEAD] 142,        /* Previous owner died */
    [ESTRPIPE] 143,          /* Streams pipe error */

	[EWOULDBLOCK] 11         /* EWOULDBLOCK has same errno with EAGAIN*/
};


int os_to_newlib_errno(int os_errno)
{
	return _os_to_newlib_errno[os_errno];
}




/********* Socket constants translate *********/
#include <sys/socket.h>

int newlib_to_os_socket_types(int newlib_socket_constant)
{
    switch(newlib_socket_constant)
    {
        /* Reverse mapping for socket types */
        case 1:
            return SOCK_STREAM; /* stream socket */
        case 2:
            return SOCK_DGRAM; /* datagram socket */
        case 3:
            return SOCK_RAW; /* raw-protocol interface */
        case 4:
            return SOCK_RDM; /* reliably-delivered message */
        case 5:
            return SOCK_SEQPACKET; /* sequenced packet stream */

        default:
            return -1; /* Unknown or unsupported newlib socket constant */
    }
}

int newlib_to_os_socket_option_flags(int newlib_socket_constant)
{
    switch(newlib_socket_constant)
    {
        /* Reverse mapping for option flags per-socket */
        case 0x0001:
            return SO_DEBUG; /* turn on debugging info recording */
        case 0x0002:
            return SO_ACCEPTCONN; /* socket has had listen() */
        case 0x0004:
            return SO_REUSEADDR; /* allow local address reuse */
        case 0x0008:
            return SO_KEEPALIVE; /* keep connections alive */
        case 0x0010:
            return SO_DONTROUTE; /* just use interface addresses */
        case 0x0020:
            return SO_BROADCAST; /* permit sending of broadcast msgs */
        // case 0x0040:
        //     return SO_USELOOPBACK; /* bypass hardware when possible */
        case 0x0080:
            return SO_LINGER; /* linger on close if data present */
        case 0x0100:
            return SO_OOBINLINE; /* leave received OOB data in line */
        case 0x0200:
            return SO_REUSEPORT; /* allow local address & port reuse */
        case 0x0400:
            return SO_TIMESTAMP; /* timestamp received dgram traffic */
        // case 0x1000:
        //     return SO_ACCEPTFILTER; /* there is an accept filter */
        
        /* Reverse mapping for additional options, not kept in so_options */
        case 0x1001:
            return SO_SNDBUF; /* send buffer size */
        case 0x1002:
            return SO_RCVBUF; /* receive buffer size */
        case 0x1003:
            return SO_SNDLOWAT; /* send low-water mark */
        case 0x1004:
            return SO_RCVLOWAT; /* receive low-water mark */
        case 0x1005:
            return SO_SNDTIMEO; /* send timeout */
        case 0x1006:
            return SO_RCVTIMEO; /* receive timeout */
        case 0x1007:
            return SO_ERROR; /* get error status and clear */
        case 0x1008:
            return SO_TYPE; /* get socket type */
        // case 0x1009:
        //     return SO_PRIVSTATE; /* get/deny privileged state - Commented as it's not in the provided constants */
        
        default:
            return -1; /* Unknown or unsupported newlib socket constant */
    }
}

int newlib_to_os_socket_option_level(int newlib_socket_constant)
{
    switch(newlib_socket_constant)
    {
        case 0xffff:
            return SOL_SOCKET;
        default:
            return -1;
    }
}