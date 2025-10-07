// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RFILES_GDRIVE_LIMITED_H
#define RFILES_GDRIVE_LIMITED_H

#include <sap_headers.h>
#include <palddk.h>
#include <rfiles_header.h>

#ifndef __ASSEMBLY__

// Forward declarations
struct gdrive_poll_state;
typedef struct gdrive_poll_state gdrive_tokens_poll_state_t;

typedef struct {
    char *device_code;
    char *user_code;
    char *verification_uri;
    char *verification_uri_with_code;
    char *verification_uri_complete;  // may be NULL
    int   interval;                   // seconds
    int   expires_in;                 // seconds
} device_user_code_t;

extern SAP_STATUS gdrive_init(void);
extern void gdrive_fini(void);

/// @brief Blocking function for getting GDrive device code and user code.
/// @param duc Struct returned from <gdrive_login_query_device_user_code>
/// @return 
extern SAP_STATUS gdrive_login_query_device_user_code(device_user_code_t *out_duc);

/// @brief Blocking function for GDrive poll for login tokens.
/// @param duc Struct returned from <gdrive_login_query_device_user_code>.
/// @return 
extern SAP_STATUS gdrive_login_poll_tokens(const device_user_code_t *duc);

extern gdrive_tokens_poll_state_t* gdrive_login_tokens_poll_state_init(const device_user_code_t *duc);
extern void gdrive_login_tokens_poll_state_fini(gdrive_tokens_poll_state_t* S);

/// @brief Fast function: Polling once the login tokens of GDrive. The function returns even if the user has not 
/// logged in thru their mobile phones. The caller must first call <gdrive_login_tokens_poll_state_init> to init the 
/// poll state and call <gdrive_login_tokens_poll_state_fini> aftrer no longer polls.
/// @param duc 
/// @param S 
/// @return 
extern SAP_STATUS gdrive_login_poll_tokens_once(const device_user_code_t *duc, gdrive_tokens_poll_state_t* S);

/// @brief Blocking function for GDrive login, including (1) getting device code and user code, and (2) poll for login
/// tokens.
/// @param username [Optional] Not needed for input-limited device login to GDrive
/// @param password [Optional] Not needed for input-limited device login to GDrive
/// @return 
extern SAP_STATUS gdrive_login(char* username, char* password);

/********* Debug Functions *********/
void gdrive_test(void);

#endif // __ASSEMBLY__
#endif // RFILES_GDRIVE_LIMITED_H