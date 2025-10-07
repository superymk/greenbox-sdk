// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>

#include <rdesktop_header.h>

static char _username[RD_USERNAME_LENGTH_MAX] = {0};
static char _password[RD_PASSWORD_LENGTH_MAX] = {0};

void cred_username_get(char* out_str)
{
	strncpy(out_str, _username, strnlen(_username, RD_USERNAME_LENGTH_MAX));
	
	// Clear the username from memory
	memset(_username, 0, RD_USERNAME_LENGTH_MAX);
}

void cred_username_set(const char* username)
{
	strncpy(_username, username, RD_USERNAME_LENGTH_MAX);
}

void cred_password_get(char* out_str)
{
	strncpy(out_str, _password, strnlen(_password, RD_PASSWORD_LENGTH_MAX));
	
	// Clear the password from memory
	memset(_password, 0, RD_PASSWORD_LENGTH_MAX);
}

void cred_password_set(const char* password)
{
	strncpy(_password, password, RD_PASSWORD_LENGTH_MAX);
}