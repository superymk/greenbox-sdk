// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file The definitions in this file must be the same with 
/// <secapp_libs/newlib/newlib-4.5.0/newlib/libc/sys/secos/gloss/_libc_cmds.h>

#ifndef SECAPP_LIBC_CMDS_H
#define SECAPP_LIBC_CMDS_H

// Commands
#define CMD_FILE							(0)
#define CMD_FILE_OPEN 						(CMD_FILE + 1)
#define CMD_FILE_READ 						(CMD_FILE + 2)
#define CMD_FILE_WRITE 						(CMD_FILE + 3)
#define CMD_FILE_CLOSE 						(CMD_FILE + 4)
#define CMD_FILE_FSTAT 						(CMD_FILE + 5)
#define CMD_FILE_STAT						(CMD_FILE + 6)
#define CMD_FILE_ISATTY						(CMD_FILE + 7)
#define CMD_FILE_SELECT						(CMD_FILE + 8)
#define CMD_FILE_FCNTL						(CMD_FILE + 9)
#define CMD_FILE_LSEEK						(CMD_FILE + 10)
#define CMD_FILE_POLL						(CMD_FILE + 11)
#define CMD_FILE_RENAME				        (CMD_FILE + 12)
#define CMD_FILE_ACCESS				        (CMD_FILE + 13)

#define CMD_FILEDIR							(70)
#define CMD_FILEDIR_MKDIR					(CMD_FILEDIR + 1)
#define CMD_FILEDIR_OPENDIR					(CMD_FILEDIR + 2)
#define CMD_FILEDIR_READDIR					(CMD_FILEDIR + 3)
#define CMD_FILEDIR_CLOSEDIR				(CMD_FILEDIR + 4)

#define CMD_STAT							(80)
#define CMD_STAT_CHMOD 						(CMD_STAT + 1)
#define CMD_STAT_FCHMOD 					(CMD_STAT + 2)

#define CMD_SOCK							(100)
#define CMD_SOCK_ACCEPT						(CMD_SOCK + 1)
#define CMD_SOCK_BIND						(CMD_SOCK + 2)
#define CMD_SOCK_CONNECT					(CMD_SOCK + 3)
#define CMD_SOCK_GETPEERNAME				(CMD_SOCK + 4)
#define CMD_SOCK_GETSOCKNAME				(CMD_SOCK + 5)
#define CMD_SOCK_RECV						(CMD_SOCK + 6)
#define CMD_SOCK_SEND						(CMD_SOCK + 7)
#define CMD_SOCK_GETSOCKOPT					(CMD_SOCK + 8)
#define CMD_SOCK_SETSOCKOPT					(CMD_SOCK + 9)
#define CMD_SOCK_SOCKET						(CMD_SOCK + 10)
#define CMD_SOCK_SOCKETPAIR					(CMD_SOCK + 11)
#define CMD_SOCK_LISTEN					    (CMD_SOCK + 12)
#define CMD_SOCK_SHUTDOWN				    (CMD_SOCK + 13)
#define CMD_SOCK_SOCKATMARK				    (CMD_SOCK + 14)
#define CMD_SOCK_RECVFROM				    (CMD_SOCK + 15)

#define CMD_INET							(150)
#define CMD_INET_ADDR						(CMD_INET + 1)
#define CMD_INET_NTOA						(CMD_INET + 2)
#define CMD_INET_PTON						(CMD_INET + 3)
#define CMD_INET_NTOP						(CMD_INET + 4)

#define CMD_IOCTL                           (180)
#define CMD_IOCTL_ARG1_RET                  (CMD_IOCTL + 1)

#define CMD_NETDB							(200)
#define CMD_NETDB_ENDHOSTENT 				(CMD_NETDB + 1)
#define CMD_NETDB_ENDPROTOENT 				(CMD_NETDB + 2)
#define CMD_NETDB_ENDSERVENT 				(CMD_NETDB + 3)
#define CMD_NETDB_GETHOSTBYADDR				(CMD_NETDB + 4)
#define CMD_NETDB_GETHOSTBYNAME				(CMD_NETDB + 5)
#define CMD_NETDB_GETPROTOBYNAME			(CMD_NETDB + 6)
#define CMD_NETDB_GETPROTOBYNUMBER			(CMD_NETDB + 7)
#define CMD_NETDB_GETPROTOENT				(CMD_NETDB + 8)
#define CMD_NETDB_GETSERVBYNAME				(CMD_NETDB + 9)
#define CMD_NETDB_GETSERVBYPORT				(CMD_NETDB + 10)
#define CMD_NETDB_GETSERVENT				(CMD_NETDB + 11)
#define CMD_NETDB_SETHOSTENT				(CMD_NETDB + 12)
#define CMD_NETDB_SETPROTOENT				(CMD_NETDB + 13)
#define CMD_NETDB_SETSERVENT				(CMD_NETDB + 14)
#define CMD_NETDB_GETADDRINFO				(CMD_NETDB + 15)
#define CMD_NETDB_GETNAMEINFO				(CMD_NETDB + 16)

#define CMD_TIME							(250)
#define CMD_TIME_GETTIMEOFDAY				(CMD_TIME + 1)
#define CMD_TIME_TIMES				        (CMD_TIME + 2)

#define CMD_SYSLOG							(300)
#define CMD_SYSLOG_CLOSE				    (CMD_SYSLOG + 1)
#define CMD_SYSLOG_OPEN				        (CMD_SYSLOG + 2)
#define CMD_SYSLOG_SETLOGMASK				(CMD_SYSLOG + 3)
#define CMD_SYSLOG_SYSLOG				    (CMD_SYSLOG + 4)

#endif // SECAPP_LIBC_CMDS_H