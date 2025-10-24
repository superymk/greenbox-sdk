// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#define USE_LIBOAUTH2_HTTP

#include "ssl_ca.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "utils_fs.h"

#ifdef _WIN32
/* If libcurl was built with Schannel, Windows System Store is used automatically. */
#endif

/* ---------- Internals ---------- */

/* Resolved CA file path (for diagnostics / fallback) */
static char *g_cafile = NULL; /* malloc'd, may be NULL */

/* Optional CA directory path (hashed dir on Linux); used only as fallback */
static char *g_capath = NULL; /* malloc'd, may be NULL */

/* In-memory CA blob (preferred) */
static struct curl_blob g_cablob;     /* points to g_cabuf when present */
static unsigned char *g_cabuf = NULL; /* malloc'd buffer holding the CA file */
static size_t g_cabuf_len = 0;

static int g_inited = 0;

/* ---------- Utilities ---------- */

static bool file_exists(const char *p)
{
    struct stat st;
    return (p && *p && stat(p, &st) == 0 && (st.st_mode & S_IFREG));
}
static bool dir_exists(const char *p)
{
    struct stat st;
    return (p && *p && stat(p, &st) == 0 && (st.st_mode & S_IFDIR));
}

static char *dup_str(const char *s)
{
    if (!s)
        return NULL;
    size_t n = strlen(s);
    char *r = (char *)malloc(n + 1);
    if (!r)
        return NULL;
    memcpy(r, s, n + 1);
    return r;
}

static int read_file_all(const char *path, unsigned char **out_buf, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        return -1;
    }
    long sz = ftell(f);
    if (sz < 0)
    {
        fclose(f);
        return -1;
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return -1;
    }

    unsigned char *buf = (unsigned char *)malloc((size_t)sz);
    if (!buf)
    {
        fclose(f);
        return -1;
    }

    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz)
    {
        free(buf);
        return -1;
    }

    *out_buf = buf;
    *out_len = (size_t)sz;
    return 0;
}

// /* Common Linux/Unix locations (order matters) */
// static const char *const CAFILE_CANDIDATES[] = {
//     "/etc/ssl/certs/ca-certificates.crt", /* Debian/Ubuntu */
//     "/etc/pki/tls/certs/ca-bundle.crt",   /* RHEL/CentOS/Fedora */
//     "/etc/ssl/cert.pem",                  /* Alpine/musl, macOS Homebrew */
//     "/etc/ssl/ca-bundle.pem",             /* SUSE/openSUSE */
//     NULL};
// static const char *const CAPATH_CANDIDATES[] = {
//     "/etc/ssl/certs",
//     "/etc/pki/tls/certs",
//     NULL};

// /* Detect Unix-ish CA file & dir; Windows Schannel is handled by curl itself. */
// static void detect_unix_ca(void)
// {
//     if (!g_cafile)
//     {
//         for (int i = 0; CAFILE_CANDIDATES[i]; i++)
//         {
//             if (file_exists(CAFILE_CANDIDATES[i]))
//             {
//                 g_cafile = dup_str(CAFILE_CANDIDATES[i]);
//                 break;
//             }
//         }
//     }
//     if (!g_capath)
//     {
//         for (int i = 0; CAPATH_CANDIDATES[i]; i++)
//         {
//             if (dir_exists(CAPATH_CANDIDATES[i]))
//             {
//                 g_capath = dup_str(CAPATH_CANDIDATES[i]);
//                 break;
//             }
//         }
//     }
// }

// /* If curl is built with Schannel on Windows, CAINFO options are typically ignored. */
// __attribute__((unused)) static int curl_uses_schannel(void)
// {
// #ifdef _WIN32
//     const curl_version_info_data *vi = curl_version_info(CURLVERSION_NOW);
//     if (vi && vi->ssl_version && strstr(vi->ssl_version, "Schannel"))
//         return 1;
// #endif
//     return 0;
// }

/********* Public Functions *********/
SAP_STATUS sslca_init(const char *certs_filepath)
{
    SAP_STATUS status = SUCCESS;

    // Check: params must be valid
    if (!certs_filepath)
        return EINVAL;

    if (g_inited)
        return EINVAL;
    g_inited = 1;

    // #ifndef _WIN32
    //     /* Autodetect on Unix-like systems */
    //     detect_unix_ca();
    // #else
    //     /* On Windows: if not Schannel (e.g., OpenSSL build), user may still have a CA file
    //        in a common path (e.g., Git for Windows bundle). We can try typical Unix paths
    //        too in case of MSYS2/MinGW environments. */
    //     detect_unix_ca();
    // #endif

    // Check: <certs_filepath> must be good.
    g_cafile = dup_str(certs_filepath);
    if (!file_exists(g_cafile))
    {
        SAP_ERR("SSL-CA Init Error: Invalid CA file (%s)!\n", g_cafile);
        status = EINVAL;
        goto cafile_err;
    }

    g_capath = path_dirname(certs_filepath);
    if (!dir_exists(g_capath))
    {
        SAP_ERR("SSL-CA Init Error: Invalid CA Directory (%s)!\n", g_capath);
        status = EINVAL;
        goto capath_err;
    }

    /* Load the CA file into memory if we have a path */
    if (g_cafile)
    {
        if (read_file_all(g_cafile, &g_cabuf, &g_cabuf_len) == 0)
        {
            /* Prepare curl_blob describing the in-memory CA bundle */
            memset(&g_cablob, 0, sizeof(g_cablob));
            g_cablob.data = g_cabuf;
            g_cablob.len = g_cabuf_len;
            g_cablob.flags = CURL_BLOB_COPY;
            /* We keep ownership of g_cabuf and free it on process exit if needed.
               libcurl does not free this memory. */
        }
        else
        {
            /* If reading fails, fall back to path-based verification later. */
            if (g_cabuf)
            {
                free(g_cabuf);
                g_cabuf = NULL;
                g_cabuf_len = 0;
            }
        }
    }

    // On success
    return SUCCESS;

capath_err:
    if (g_capath)
    {
        free(g_capath);
        g_capath = NULL;
    }

cafile_err:
    if (g_cafile)
    {
        free(g_cafile);
        g_cafile = NULL;
    }

    return status;
}

void sslca_fini(void)
{
    // Clear <g_cablob>
    memset(&g_cablob, 0, sizeof(g_cablob));
    g_cabuf_len = 0;
    if (g_cabuf)
    {
        free(g_cabuf);
        g_cabuf = NULL;
    }

    // Clear <g_cafile> and <g_capath>
    if (g_cafile)
    {
        free(g_cafile);
        g_cafile = NULL;
    }

    if (g_capath)
    {
        free(g_capath);
        g_capath = NULL;
    }
}

__attribute__((unused)) static void sslca_curl_set_no_ssl_verify(CURL *easy)
{
    SAP_ERR("!!! Warning: Disabled SSL verification !!!\n");

    curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
}

void sslca_curl_set(CURL *easy)
{
    if (!easy)
        return;

    // // [TODO][Issue 186] Disabled SSL verification temorally. 
    // sslca_curl_set_no_ssl_verify(easy);
    // return;

    /* If you want to see which TLS backend you’re using */
    const curl_version_info_data *vi = curl_version_info(CURLVERSION_NOW);
    SAP_INFO("SSL backend: %s\n", vi && vi->ssl_version ? vi->ssl_version : "(unknown)");

    /* Prefer in-memory bundle */
    if (g_cabuf && g_cabuf_len > 0)
    {
        CURLcode rc = curl_easy_setopt(easy, CURLOPT_CAINFO_BLOB, &g_cablob);
        if (rc != CURLE_OK)
        {
            SAP_ERR("CURLOPT_CAINFO_BLOB failed: %s\n", curl_easy_strerror(rc));
        }
        else
        {
            SAP_INFO("SSL-CA: Using CAINFO_BLOB (%lu bytes)\n", g_cabuf_len);
        }

        /* Strong verification */
        curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 2L);
        return;
    }

    /* Fallback: file path */
    if (g_cafile && *g_cafile)
    {
        curl_easy_setopt(easy, CURLOPT_CAINFO, g_cafile);
        curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 2L);
        SAP_INFO("SSL-CA: Using CAINFO path: %s\n", g_cafile);
    }

    /* CAPATH only helps on Unix when directory is c_rehash-hashed */
    if (g_capath && *g_capath) {
        curl_easy_setopt(easy, CURLOPT_CAPATH, g_capath);
    }
}

const char *sslca_get_cafile(void) { return g_cafile; }
const char *sslca_get_capath(void) { return g_capath; }
