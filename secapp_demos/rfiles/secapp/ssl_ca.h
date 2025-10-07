// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef RFILES_SSL_CA_H
#define RFILES_SSL_CA_H

#include <sap_headers.h>
#include <palddk.h>

#include <curl/curl.h>

#ifndef __ASSEMBLY__

/*
 * sslca_init()
 *   Detect system CA file/dir (Linux/Unix) or defer to platform defaults (Windows Schannel).
 *   Exports env vars (SSL_CERT_FILE, CURL_CA_BUNDLE, SSL_CERT_DIR) if they are not already set,
 *   so liboauth2’s internal libcurl will use the right trust store.
 *
 *   Call this **once at startup**, before any OAuth/device-code or curl work.
 */
void sslca_init(void);

/*
 * sslca_curl_set(CURL *easy)
 *   Apply CA settings to your libcurl easy handle:
 *     - On Linux/Unix: sets CURLOPT_CAINFO / CURLOPT_CAPATH if discovered.
 *     - On Windows Schannel: does nothing (Schannel uses system store automatically).
 *     - On Windows OpenSSL/LibreSSL: if env provides a bundle/dir, applies it.
 *
 *   Call this after curl_easy_init() and before curl_easy_perform().
 */
void sslca_curl_set(CURL *easy);

/*
 * Optional getters (may return NULL). Useful for diagnostics/logging.
 */
const char *sslca_get_cafile(void);
const char *sslca_get_capath(void);

#endif // __ASSEMBLY__
#endif // RFILES_SSL_CA_H