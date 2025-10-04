/*	$OpenBSD: malloc.h,v 1.125 2023/07/03 06:45:44 guenther Exp $	*/
/*	$NetBSD: malloc.h,v 1.39 1998/07/12 19:52:01 augustss Exp $	*/

/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)malloc.h	8.5 (Berkeley) 5/3/95
 */

#ifndef _SYS_MALLOC_H_
#define	_SYS_MALLOC_H_

#include <sys/queue.h>

#define KERN_MALLOC_BUCKETS	1
#define KERN_MALLOC_BUCKET	2
#define KERN_MALLOC_KMEMNAMES	3
#define KERN_MALLOC_KMEMSTATS	4
#define KERN_MALLOC_MAXID	5

#define CTL_KERN_MALLOC_NAMES { \
	{ 0, 0 }, \
	{ "buckets", CTLTYPE_STRING }, \
	{ "bucket", CTLTYPE_NODE }, \
	{ "kmemnames", CTLTYPE_STRING }, \
	{ "kmemstat", CTLTYPE_NODE }, \
}

/*
 * flags to malloc
 */
#define	M_WAITOK	0x0001
#define	M_NOWAIT	0x0002
#define	M_CANFAIL	0x0004
#define	M_ZERO		0x0008

/*
 * Types of memory to be allocated
 */
#define	M_FREE		0	/* should be on free list */
/* 1 - free */
#define	M_DEVBUF	2	/* device driver memory */
/* 3 - free */
#define	M_PCB		4	/* protocol control blocks */
#define	M_RTABLE	5	/* routing tables */
#define	M_PF		6	/* packet filter structures */
/* 7 - free */
/* 8 - free */
#define	M_IFADDR	9	/* interface addresses */
#define	M_IFGROUP	10	/* interface groups */
#define	M_SYSCTL	11	/* sysctl persistent buffers */
#define	M_COUNTERS	12	/* per-CPU counters via counters_alloc(9) */
/* 13 - free */
#define	M_IOCTLOPS	14	/* ioctl data buffers */
/* 15-18 - free */
#define	M_IOV		19	/* large IOVs */
#define	M_MOUNT		20	/* VFS mount structs */
/* 21 - free */
#define	M_NFSREQ	22	/* NFS request headers */
#define	M_NFSMNT	23	/* NFS mount structures */
#define	M_LOG		24	/* messages in kernel log stash */
#define	M_VNODE		25	/* Dynamically allocated vnodes */
/* 26 - free */
#define	M_DQUOT		27	/* UFS quota entries */
#define	M_UFSMNT	28	/* UFS mount structures */
#define	M_SHM		29	/* SVID compatible shared memory segments */
#define	M_VMMAP		30	/* VM map structures */
#define	M_SEM		31	/* SVID compatible semaphores */
#define	M_DIRHASH	32	/* UFS directory hash structures */
#define	M_ACPI		33	/* ACPI structures */
#define	M_VMPMAP	34	/* VM pmap data */
/* 35-38 - free */
#define	M_FILEDESC	39	/* open file descriptor tables */
#define	M_SIGIO		40	/* sigio structures */
#define	M_PROC		41	/* proc structures */
#define	M_SUBPROC	42	/* proc sub-structures */
/* 43-45 - free */
#define	M_MFSNODE	46	/* MFS vnode private part */
/* 47-48 - free */
#define	M_NETADDR	49	/* export host address structures */
#define	M_NFSSVC	50	/* NFS server structures */
/* 51 - free */
#define	M_NFSD		52	/* NFS server daemon structures */
#define	M_IPMOPTS	53	/* internet multicast options */
#define	M_IPMADDR	54	/* internet multicast addresses */
#define	M_IFMADDR	55	/* link-level multicast addresses */
#define	M_MRTABLE	56	/* multicast routing tables */
#define	M_ISOFSMNT	57	/* ISOFS mount structures */
#define	M_ISOFSNODE	58	/* ISOFS vnode private part */
#define	M_MSDOSFSMNT	59	/* MSDOS FS mount structures */
#define	M_MSDOSFSFAT	60	/* MSDOS FS FAT tables */
#define	M_MSDOSFSNODE	61	/* MSDOS FS vnode private part */
#define	M_TTYS		62	/* allocated tty structures */
#define	M_EXEC		63	/* argument lists & other mem used by exec */
#define	M_MISCFSMNT	64	/* miscellaneous FS mount structures */
#define	M_FUSEFS	65	/* FUSE FS mount structures */
/* 66-73 - free */
#define	M_PFKEY		74	/* pfkey data */
#define	M_TDB		75	/* transforms database */
#define	M_XDATA		76	/* IPsec data */
/* 77 - free */
#define	M_PAGEDEP	78	/* file page dependencies */
#define	M_INODEDEP	79	/* inode dependencies */
#define	M_NEWBLK	80	/* new block allocation */
/* 81-82 - free */
#define	M_INDIRDEP	83	/* indirect block dependencies */
/* 84-91 - free */
#define	M_VMSWAP	92	/* VM swap structures */
/* 93-97 - free */
#define	M_UVMAMAP	98	/* UVM amap and related */
#define	M_UVMAOBJ	99	/* UVM aobj and related */
/* 100 - free */
#define	M_USB		101	/* USB general */
#define	M_USBDEV	102	/* USB device driver */
#define	M_USBHC		103	/* USB host controller */
#define	M_WITNESS	104	/* witness(4) memory */
#define	M_MEMDESC	105	/* memory range */
/* 106-107 - free */
#define	M_CRYPTO_DATA	108	/* crypto(9) data buffers */
/* 109 - free */
#define	M_CREDENTIALS	110	/* ipsec(4) related credentials */
/* 111-122 - free */

/* KAME IPv6 */
#define	M_IP6OPT	123	/* IPv6 options */
#define	M_IP6NDP	124	/* IPv6 Neighbor Discovery structures */
/* 125-126 - free */
#define	M_TEMP		127	/* miscellaneous temporary data buffers */

#define	M_NTFSMNT	128	/* NTFS mount structures */
#define	M_NTFSNTNODE	129	/* NTFS ntnode information */
#define	M_NTFSFNODE	130	/* NTFS fnode information */
#define	M_NTFSDIR	131	/* NTFS directory buffers */
#define	M_NTFSNTHASH	132	/* NTFS ntnode hash tables */
#define	M_NTFSNTVATTR	133	/* NTFS file attribute information */
#define	M_NTFSRDATA	134	/* NTFS resident data */
#define	M_NTFSDECOMP	135	/* NTFS decompression temporary storage */
#define	M_NTFSRUN	136	/* NTFS vrun storage */

#define	M_KEVENT	137	/* kqueue(2) data structures */

	/*		138	   free */
#define	M_SYNCACHE	139	/* SYN cache hash array */

#define	M_UDFMOUNT	140	/* UDF mount structures */
#define	M_UDFFENTRY	141	/* UDF file entries */
#define	M_UDFFID	142	/* UDF file IDs */

	/*		143	   free */

#define	M_AGP		144	/* AGP memory */

#define	M_DRM		145	/* Direct Rendering Manager */

#define	M_LAST		146	/* Must be last type + 1 */


void	*malloc(size_t, int, int);
#endif /* !_SYS_MALLOC_H_ */
