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

#ifndef RDESKTOP_USER_CREDENTIAL_H
#define RDESKTOP_USER_CREDENTIAL_H

#define RD_USERNAME_LENGTH_MAX	(64)
#define RD_PASSWORD_LENGTH_MAX (64)

#ifndef __ASSEMBLY__

extern void cred_username_get(char* out_str);

extern void cred_username_set(const char* username);

extern void cred_password_get(char* out_str);

extern void cred_password_set(const char* password);

#endif // __ASSEMBLY__
#endif // RDESKTOP_USER_CREDENTIAL_H