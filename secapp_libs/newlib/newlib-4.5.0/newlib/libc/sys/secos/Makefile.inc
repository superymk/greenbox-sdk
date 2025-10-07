# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS) -I$(SECOS_PUBLIC_HEADERS)

# ./gloss
libc_a_SOURCES += %D%/gloss/__popcountdi2.c %D%/gloss/__udivmoddi4.c %D%/gloss/__udivmodti4.c %D%/gloss/divdi3.S %D%/gloss/udivdi3.S %D%/gloss/umoddi3.S 

libc_a_SOURCES += %D%/gloss/i386_cpu2.S
libc_a_SOURCES += %D%/gloss/amd64_cpu2.S

libc_a_SOURCES += %D%/gloss/lock.c %D%/gloss/gloss_mm.c %D%/gloss/gloss_sp.c %D%/gloss/gloss_stdio.c %D%/gloss/gloss_semaphore.c %D%/gloss/stl_dlist.c %D%/gloss/ids.c %D%/gloss/fd.c %D%/gloss/syslog.c %D%/gloss/stat.c %D%/gloss/ioctl.c 
libc_a_SOURCES += %D%/gloss/_libc.c %D%/gloss/_dbg_print.c %D%/gloss/_dbg_print_bcd.c %D%/gloss/syscalls.c %D%/gloss/unistd.c %D%/gloss/debug.c

# ./security
libc_a_SOURCES += %D%/security/tlsf.c %D%/security/mm.c


# ./net
libc_a_SOURCES += %D%/net/netdb.c %D%/net/socket.c %D%/net/inet.c


# ./ipc
libc_a_SOURCES += %D%/ipc/comm_sp_app.c %D%/ipc/comm_sp_sp.c %D%/ipc/cmp.c %D%/ipc/varbuf.c


# ./
libc_a_SOURCES += %D%/sap_syscalls.c %D%/sap_loader.c

libc_a_SOURCES += %D%/i386_signal_entry.S %D%/i386_sap_syscalls.c
libc_a_SOURCES += %D%/amd64_signal_entry.S %D%/amd64_sap_syscalls.c