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

#ifndef SECAPP_VAR_H
#define SECAPP_VAR_H

#ifndef __ASSEMBLY__

// Secure foreach
// we should use it as much as possible for every for statement
#define FOREACH_S(var_name, m1, m2, initial, step) for(var_name = (initial); (var_name < (m1)) && (var_name < (m2)); var_name += (step))

#endif // __ASSEMBLY__
#endif // SECAPP_VAR_H