/*	$OpenBSD: endian.h,v 1.25 2014/12/21 04:49:00 guenther Exp $	*/

/*-
 * Copyright (c) 1997 Niklas Hallqvist.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Public definitions for little- and big-endian systems.
 * This file should be included as <endian.h> in userspace and as
 * <sys/endian.h> in the kernel.
 *
 * System headers that need endian information but that can't or don't
 * want to export the public names here should include <sys/_endian.h>
 * and use the internal names: _BYTE_ORDER, _*_ENDIAN, etc.
 */

#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

#include <sys/cdefs.h>
#include <sys/_endian.h>

/* Public names */
#define LITTLE_ENDIAN	_LITTLE_ENDIAN
#define BIG_ENDIAN	_BIG_ENDIAN
#define PDP_ENDIAN	_PDP_ENDIAN
#define BYTE_ORDER	_BYTE_ORDER

/*
 * These are specified to be function-like macros to match the spec
 */
#define htobe16(x)	__htobe16(x)
#define htobe32(x)	__htobe32(x)
#define htobe64(x)	__htobe64(x)
#define htole16(x)	__htole16(x)
#define htole32(x)	__htole32(x)
#define htole64(x)	__htole64(x)

/* POSIX names */
#define be16toh(x)	__htobe16(x)
#define be32toh(x)	__htobe32(x)
#define be64toh(x)	__htobe64(x)
#define le16toh(x)	__htole16(x)
#define le32toh(x)	__htole32(x)
#define le64toh(x)	__htole64(x)

/* original BSD names */
#define betoh16(x)	__htobe16(x)
#define betoh32(x)	__htobe32(x)
#define betoh64(x)	__htobe64(x)
#define letoh16(x)	__htole16(x)
#define letoh32(x)	__htole32(x)
#define letoh64(x)	__htole64(x)



#endif /* _SYS_ENDIAN_H_ */