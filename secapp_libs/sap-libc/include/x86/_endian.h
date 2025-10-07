// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// @file This file includes the headers for general SecApps. Most SecApps should include this header only to function.
// This file does not contain any headers of IOSM clients, because those clients require additional headers exported by
// IOSMs. General SecApps do not need to include IOSM headers. Thus, SecApps using IOSM clients should include IOSM 
// headers by themselves.

#ifndef SECAPP__ENDIAN_H
#define SECAPP__ENDIAN_H




#ifndef __ASSEMBLY__


/*
 * On little-endian machines that can handle unaligned accesses
 * (e.g. i386) these macros can be replaced by the following.
 */
#define UGETW(w) (*(u_int16_t *)(w))
#define USETW(w,v) (*(u_int16_t *)(w) = (v))
#define UGETDW(w) (*(u_int32_t *)(w))
#define USETDW(w,v) (*(u_int32_t *)(w) = (v))


#endif // __ASSEMBLY__
#endif // SECAPP__ENDIAN_H