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

#ifndef SUBSYS_OPENBSD_HEADER_MISSING_H
#define SUBSYS_OPENBSD_HEADER_MISSING_H

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif 

#ifndef max
#define max(a, b) (a < b ? b : a)
#endif

#endif // SUBSYS_OPENBSD_HEADER_MISSING_H