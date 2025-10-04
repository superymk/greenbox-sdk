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

#ifndef LIBC_BACKEND_INFO_H
#define LIBC_BACKEND_INFO_H

#ifndef __ASSEMBLY__

#define ERR(format, ...) \
	do{	\
		printf("[LIBC-BACKEND][ERR] "format, ## __VA_ARGS__); } while(0)

#endif // __ASSEMBLY__
#endif // LIBC_BACKEND_INFO_H