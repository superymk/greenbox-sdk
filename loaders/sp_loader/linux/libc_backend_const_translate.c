// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

int newlib_to_os_socket_send_recv_flags(int newlib_socket_constant)
{
    /* Known newlib flag bits we handle */
    const int KNOWN =
        0x1     /* MSG_OOB       */
        | 0x2    /* MSG_PEEK      */
        | 0x4    /* MSG_DONTROUTE */
        | 0x8    /* MSG_EOR       */
        | 0x10   /* MSG_TRUNC     */
        | 0x20   /* MSG_CTRUNC    */
        | 0x40   /* MSG_WAITALL   */
        | 0x80    /* MSG_DONTWAIT  */
        // | 0x100  | /* MSG_EOF       */
        // | 0x8000  /* MSG_COMPAT    */
        ;

    /* Reject unknown bits to avoid silently dropping flags */
    if (newlib_socket_constant & ~KNOWN)
    {
        APP_ERR("[Socket Send/Recv] Flags (0x%08X) have unknown bits!\n", (uint32_t)newlib_socket_constant);
        return -1;
    }

    int os_flags = 0;

    if (newlib_socket_constant & 0x1)    os_flags |= MSG_OOB;
    if (newlib_socket_constant & 0x2)    os_flags |= MSG_PEEK;
    if (newlib_socket_constant & 0x4)    os_flags |= MSG_DONTROUTE; /* mainly for send */
    if (newlib_socket_constant & 0x8)    os_flags |= MSG_EOR;
    if (newlib_socket_constant & 0x10)   os_flags |= MSG_TRUNC;
    if (newlib_socket_constant & 0x20)   os_flags |= MSG_CTRUNC;
    if (newlib_socket_constant & 0x40)   os_flags |= MSG_WAITALL;
    if (newlib_socket_constant & 0x80)   os_flags |= MSG_DONTWAIT;
    // if (newlib_socket_constant & 0x100)  os_flags |= MSG_EOF;
    // if (newlib_socket_constant & 0x8000) os_flags |= MSG_COMPAT;

    return os_flags;
}

int newlib_to_os_socket_addr_family(int newlib_socket_constant)
{
    /* AF_MAX == 32 in the newlib snippet; allocate 0..32 */
    static const int map[33] = {
        [0]  = AF_UNSPEC,    /* PF_UNSPEC  */
        [1]  = AF_LOCAL,     /* PF_LOCAL / PF_UNIX / PF_FILE */
        [2]  = AF_INET,      /* PF_INET    */
        [3]  = AF_AX25,      /* PF_AX25    */
        [4]  = AF_IPX,       /* PF_IPX     */
        [5]  = AF_APPLETALK, /* PF_APPLETALK */
        [6]  = AF_NETROM,    /* PF_NETROM  */
        [7]  = AF_BRIDGE,    /* PF_BRIDGE  */
        [8]  = AF_ATMPVC,    /* PF_ATMPVC  */
        [9]  = AF_X25,       /* PF_X25     */
        [10] = AF_INET6,     /* PF_INET6   */
        [11] = AF_ROSE,      /* PF_ROSE    */
        [12] = AF_DECnet,    /* PF_DECnet  */
        [13] = AF_NETBEUI,   /* PF_NETBEUI */
        [14] = AF_SECURITY,  /* PF_SECURITY */
        [15] = AF_KEY,       /* PF_KEY     */
        [16] = AF_NETLINK,   /* PF_NETLINK (AF_ROUTE is alias) */
        [17] = AF_PACKET,    /* PF_PACKET  */
        [18] = AF_ASH,       /* PF_ASH     */
        [19] = AF_ECONET,    /* PF_ECONET  */
        [20] = AF_ATMSVC,    /* PF_ATMSVC  */
        /* [21] gap */
        [22] = AF_SNA,       /* PF_SNA     */
        [23] = AF_IRDA,      /* PF_IRDA    */
        [24] = AF_PPPOX,     /* PF_PPPOX   */
        [25] = AF_WANPIPE,   /* PF_WANPIPE */
        /* [26..30] gaps */
        [31] = AF_BLUETOOTH, /* PF_BLUETOOTH */
        [32] = AF_MAX        /* PF_MAX (not an actual family; treat as sentinel if ever seen) */
    };

    if (newlib_socket_constant < 0 || newlib_socket_constant >= (int)(sizeof(map)/sizeof(map[0])))
    {
        goto err;
    }

    /* Reject PF_MAX (32) and any hole that remains 0-initialized to 0 but isn't AF_UNSPEC */
    int af = map[newlib_socket_constant];

    /* If this slot is uninitialized (0) and the index is not 0 (AF_UNSPEC), treat as unknown. */
    if (af == 0 && newlib_socket_constant != 0)
        goto err;

    /* PF_MAX (32) is not a valid family to pass to socket() */
    if (newlib_socket_constant == 32)
        goto err;

    return af;

err:
    APP_ERR("[Socket Addr Family] AF (0x%08X) is invalid!\n", (uint32_t)newlib_socket_constant);
    return -1;
}

/* Reverse: OS AF_*  -> newlib PF/AF number */
int os_to_newlib_socket_addr_family(int os_af)
{
    /* Reuse the same forward map to stay consistent. */
    static const int map[33] = {
        [0]  = AF_UNSPEC,    /* PF_UNSPEC  */
        [1]  = AF_LOCAL,     /* PF_LOCAL / PF_UNIX / PF_FILE */
        [2]  = AF_INET,      /* PF_INET    */
        [3]  = AF_AX25,      /* PF_AX25    */
        [4]  = AF_IPX,       /* PF_IPX     */
        [5]  = AF_APPLETALK, /* PF_APPLETALK */
        [6]  = AF_NETROM,    /* PF_NETROM  */
        [7]  = AF_BRIDGE,    /* PF_BRIDGE  */
        [8]  = AF_ATMPVC,    /* PF_ATMPVC  */
        [9]  = AF_X25,       /* PF_X25     */
        [10] = AF_INET6,     /* PF_INET6   */
        [11] = AF_ROSE,      /* PF_ROSE    */
        [12] = AF_DECnet,    /* PF_DECnet  */
        [13] = AF_NETBEUI,   /* PF_NETBEUI */
        [14] = AF_SECURITY,  /* PF_SECURITY */
        [15] = AF_KEY,       /* PF_KEY     */
        [16] = AF_NETLINK,   /* PF_NETLINK (AF_ROUTE is alias) */
        [17] = AF_PACKET,    /* PF_PACKET  */
        [18] = AF_ASH,       /* PF_ASH     */
        [19] = AF_ECONET,    /* PF_ECONET  */
        [20] = AF_ATMSVC,    /* PF_ATMSVC  */
        /* [21] gap */
        [22] = AF_SNA,       /* PF_SNA     */
        [23] = AF_IRDA,      /* PF_IRDA    */
        [24] = AF_PPPOX,     /* PF_PPPOX   */
        [25] = AF_WANPIPE,   /* PF_WANPIPE */
        /* [26..30] gaps */
        [31] = AF_BLUETOOTH, /* PF_BLUETOOTH */
        [32] = AF_MAX        /* PF_MAX (sentinel; not valid) */
    };

#ifdef AF_ROUTE
    /* Normalize well-known alias: treat AF_ROUTE like AF_NETLINK. */
    if (os_af == AF_ROUTE)
        os_af = AF_NETLINK;
#endif

    /* Scan for the first index whose AF matches. Skip gaps and PF_MAX. */
    for (int i = 0; i < 33; ++i) {
        if (i == 32)                /* PF_MAX is not a valid family */
            continue;
        if (i != 0 && map[i] == 0)  /* holes (e.g., 21, 26..30) */
            continue;
        if (map[i] == os_af)
            return i;               /* return newlib PF/AF number */
    }

    return -1; /* unknown / unsupported AF */
}

int newlib_to_os_socket_protocol(int newlib_socket_constant)
{
    // Protocols common to RFC 1700, POSIX, and X/Open.
    return newlib_socket_constant;
}

int newlib_to_os_socket_type(int newlib_socket_constant)
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
            return newlib_socket_constant;
    }
}

int newlib_to_os_socket_shutdown_howto(int newlib_socket_constant)
{
    switch(newlib_socket_constant)
    {
        case 0:
            return SHUT_RD;
        case 1:
            return SHUT_WR;
        case 2:
            return SHUT_RDWR;
        default:
            return -1;
    }
}