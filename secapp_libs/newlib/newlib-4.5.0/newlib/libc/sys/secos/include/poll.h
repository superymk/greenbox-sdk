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

/*	$OpenBSD: poll.h,v 1.3 2003/10/29 16:41:13 deraadt Exp $ */

/*
 * Written by Theo de Raadt, Public Domain
 *
 * Typical poll() implementations expect poll.h to be in /usr/include. 
 * However this is not a convenient place for the real definitions.
 */
#include <sys/poll.h>
