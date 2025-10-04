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

#ifndef SUBSYS_OPENBSD_API_RENAME_H
#define SUBSYS_OPENBSD_API_RENAME_H

#ifndef SKIP_OPENBSD_RENAME

#undef __aligned

#define malloc(size, type, flags)  openbsd_malloc(size, type, flags)

#endif // SKIP_OPENBSD_RENAME

#endif // SUBSYS_OPENBSD_API_RENAME_H