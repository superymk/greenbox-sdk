// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <appddk.h>
#include <stdio.h>

#include <stdint.h>
#include <errno.h>

/********* Errno translate *********/
int os_to_newlib_errno(int os_errno)
{
    // Cygwin has same errno definitions with Newlib
	return os_errno;
}




/********* Socket constants translate *********/
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