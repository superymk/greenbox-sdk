// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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