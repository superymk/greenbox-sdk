// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// gdrive_limited.c (POSIX/newlib only)
// Google Drive client (Device Code OAuth2) using libcurl only for OAuth2 device flow + Drive.
// - Hardened polling: no connection or TLS session reuse; HTTP/1.1 + v4; treat 428/5xx as transient;
//   backoff with jitter; recreate handle after repeated transport errors.
// - Long-lived DNS cache + shared across handles to avoid mid-flow re-resolution on embedded stacks.

#include "gdrive_limited.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <curl/curl.h>
#include <jansson.h>

#include "ssl_ca.h"
#include "utils.h"

typedef struct
{
    CURL *C;
    struct curl_slist *hdr;
    char errbuf[CURL_ERROR_SIZE];
} post_client_t;

typedef struct gdrive_poll_state
{
    post_client_t pc;
    bool pc_inited;

    time_t start;                // when polling began
    time_t next_poll_at;         // when we’re next allowed to poll
    int poll_interval;           // current poll interval (respects slow_down)
    int consec_transport_err;    // transient transport error streak
    time_t first_transport_fail; // first transport fail wall-clock

} gdrive_tokens_poll_state_t;

#define GDRIVE_UPLOAD_DOWNLOAD_FILE_SIZE_MAX (MB(64))

const char *g_client_id = "544657840600-nmq99f57bvs4c9ojacj9il8tuj29sf5d.apps.googleusercontent.com";
const char *g_client_secret = "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";
const char *g_scope =
    "https://www.googleapis.com/auth/drive.file "
    "openid "
    "email";

// ---- Config
static const int min_poll_interval = 3; // Poll interval is >= 3 seconds.
static const char *DEVICE_CODE_URL = "https://oauth2.googleapis.com/device/code";
static const char *TOKEN_URL = "https://oauth2.googleapis.com/token";
static const char *DRIVE_API = "https://www.googleapis.com/drive/v3";
static const char *DRIVE_UPLOAD = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";

#define DEFAULT_JOIN_SEP '/'
#define MAX_CONSEC_TRANSIENT_ERR 3 // recreate handle after N consecutive transport errors
#define MAX_POLL_WALL_FAIL_SEC 240 // bail out if only transport failures for this many seconds

// ---- Global curl share (DNS cache shared across handles)
static CURLSH *g_share = NULL;

// ---- Small utilities (printf-only)
static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("ERROR: ");
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

static int is_dir(const char *p)
{
    struct stat st;
    if (stat(p, &st))
        return 0;
    return S_ISDIR(st.st_mode);
}
__attribute__((unused)) static int is_file(const char *p)
{
    struct stat st;
    if (stat(p, &st))
        return 0;
    return S_ISREG(st.st_mode);
}

static int ensure_dir(const char *path)
{
    if (is_dir(path))
        return 0;
    if (mkdir(path, 0755) == 0)
        return 0;
    if (errno == EEXIST)
        return 0;
    return -1;
}

static void sleep_seconds(int s) { sleep(s); }

static char *sdup(const char *s)
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

// Detect absolute paths (Unix, Windows drive, or UNC).
static int is_absolute_path(const char *p)
{
    if (!p || !*p)
        return 0;
    if (p[0] == '/')
        return 1;
    if (p[0] == '\\' && p[1] == '\\')
        return 1;
    if (((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')) &&
        p[1] == ':' && (p[2] == '\\' || p[2] == '/'))
        return 1;
    return 0;
}

// Join paths conservatively
static void path_join(char *out, size_t outsz, const char *a, const char *b)
{
    const char *A = (a && *a) ? a : "";
    const char *B = (b && *b) ? b : "";
    if (is_absolute_path(B) || !*A)
    {
        snprintf(out, outsz, "%s", B);
        return;
    }
    size_t la = strlen(A);
    int A_sep = (la > 0 && (A[la - 1] == '/' || A[la - 1] == '\\'));
    int B_sep = (B[0] == '/' || B[0] == '\\');
    if (A_sep || B_sep)
        snprintf(out, outsz, "%s%s", A, B);
    else
        snprintf(out, outsz, "%s/%s", A, B);
}

// Return pointer inside 'path' to basename.
static const char *basename_x(const char *path)
{
    if (!path)
        return "";
    const char *s1 = strrchr(path, '/'), *s2 = strrchr(path, '\\');
    const char *s = (!s1 ? s2 : (!s2 ? s1 : (s1 > s2 ? s1 : s2)));
    return s ? (s + 1) : path;
}

// ---- HTTP buffer
struct mem
{
    char *ptr;
    size_t len;
};
static size_t write_cb(void *data, size_t sz, size_t nm, void *ud)
{
    size_t add = sz * nm;
    struct mem *m = (struct mem *)ud;
    char *np = (char *)realloc(m->ptr, m->len + add + 1);
    if (!np)
        return 0;
    m->ptr = np;
    memcpy(m->ptr + m->len, data, add);
    m->len += add;
    m->ptr[m->len] = 0;
    return add;
}

static void curl_check(CURLcode rc, const char *where)
{
    if (rc != CURLE_OK)
        die("%s: %s", where, curl_easy_strerror(rc));
}

// ---- Common curl options to harden transport on embedded stacks
static void set_common_curl_opts(CURL *C)
{
    sslca_curl_set(C); // your CA store (impl. you provide)
    curl_easy_setopt(C, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(C, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(C, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // No connection/TLS-session reuse: match CLI `--no-keepalive` and avoid gray sockets
    curl_easy_setopt(C, CURLOPT_FORBID_REUSE, 1L);
    curl_easy_setopt(C, CURLOPT_FRESH_CONNECT, 1L);
#ifdef CURLOPT_SSL_SESSIONID_CACHE
    curl_easy_setopt(C, CURLOPT_SSL_SESSIONID_CACHE, 0L);
#endif

    // Keepalive flags won't hurt but we forbid reuse anyway
    curl_easy_setopt(C, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(C, CURLOPT_TCP_KEEPIDLE, 30L);
    curl_easy_setopt(C, CURLOPT_TCP_KEEPINTVL, 15L);

    // Fail quickly on dead/stalled connections
    curl_easy_setopt(C, CURLOPT_CONNECTTIMEOUT_MS, 30000L);
    curl_easy_setopt(C, CURLOPT_TIMEOUT_MS, 45000L);

    // curl_easy_setopt(C, CURLOPT_LOW_SPEED_LIMIT, 1L);
    // curl_easy_setopt(C, CURLOPT_LOW_SPEED_TIME, 15L);
    curl_easy_setopt(C, CURLOPT_LOW_SPEED_LIMIT, 0L);
    curl_easy_setopt(C, CURLOPT_LOW_SPEED_TIME, 0L);

    curl_easy_setopt(C, CURLOPT_UPLOAD_BUFFERSIZE, (long)(GDRIVE_UPLOAD_DOWNLOAD_FILE_SIZE_MAX));

    curl_easy_setopt(C, CURLOPT_VERBOSE, 1L);

    // Long DNS cache and share across handles
    curl_easy_setopt(C, CURLOPT_DNS_CACHE_TIMEOUT, 86400L);
    if (g_share)
        curl_easy_setopt(C, CURLOPT_SHARE, g_share);

    // UA: some middleboxes key on this
    curl_easy_setopt(C, CURLOPT_USERAGENT, "gdrive-limited/1.0 (+libcurl)");
}

// ---------- helpers for URL-encoding query params ----------
static char *escape_str(CURL *C, const char *s)
{
    if (!s)
        return sdup("");
    char *e = curl_easy_escape(C, s, 0);
    if (!e)
        return sdup("");
    char *r = sdup(e);
    curl_free(e);
    return r;
}

// ---------- Simple POST builder (string body) ----------
static void append_form_kv(CURL *C, struct mem *body, const char *k, const char *v)
{
    char *ek = curl_easy_escape(C, k ? k : "", 0);
    char *ev = curl_easy_escape(C, v ? v : "", 0);
    if (!ek || !ev)
    {
        if (ek)
            curl_free(ek);
        if (ev)
            curl_free(ev);
        die("oom");
    }
    size_t need = (body->len ? 1 : 0) + strlen(ek) + 1 + strlen(ev);
    char *nb = realloc(body->ptr, body->len + need + 1);
    if (!nb)
    {
        curl_free(ek);
        curl_free(ev);
        die("oom");
    }
    body->ptr = nb;
    char *dst = body->ptr + body->len;
    if (body->len)
        *dst++ = '&';
    strcpy(dst, ek);
    dst += strlen(ek);
    *dst++ = '=';
    strcpy(dst, ev);
    body->len += need;
    body->ptr[body->len] = 0;
    curl_free(ek);
    curl_free(ev);
}

// ---------- Reusable POST client for polling ----------
static void post_client_init(post_client_t *pc)
{
    memset(pc, 0, sizeof(*pc));
    pc->C = curl_easy_init();
    if (!pc->C)
        die("curl init");
    set_common_curl_opts(pc->C);
    pc->hdr = NULL;
    pc->hdr = curl_slist_append(pc->hdr, "Content-Type: application/x-www-form-urlencoded");
    pc->hdr = curl_slist_append(pc->hdr, "Accept: application/json");
    pc->hdr = curl_slist_append(pc->hdr, "Connection: close"); // ensure server closes
    pc->hdr = curl_slist_append(pc->hdr, "Expect:");           // avoid 100-continue
    curl_easy_setopt(pc->C, CURLOPT_HTTPHEADER, pc->hdr);
    curl_easy_setopt(pc->C, CURLOPT_POST, 1L);
    curl_easy_setopt(pc->C, CURLOPT_ERRORBUFFER, pc->errbuf);
    pc->errbuf[0] = '\0';
}

static void post_client_cleanup(post_client_t *pc)
{
    if (pc->hdr)
        curl_slist_free_all(pc->hdr);
    if (pc->C)
        curl_easy_cleanup(pc->C);
    memset(pc, 0, sizeof(*pc));
}

// Returns body (caller free) and sets *status; on transport error returns NULL and *status=0.
static char *post_client_perform(post_client_t *pc, const char *url, const char *pairs[][2], size_t npairs, long *status)
{
    if (status)
        *status = 0;
    struct mem body = {0}, resp = {0};
    for (size_t i = 0; i < npairs; i++)
        append_form_kv(pc->C, &body, pairs[i][0], pairs[i][1]);

    curl_easy_setopt(pc->C, CURLOPT_URL, url);
    curl_easy_setopt(pc->C, CURLOPT_POSTFIELDS, body.ptr ? body.ptr : "");
    curl_easy_setopt(pc->C, CURLOPT_POSTFIELDSIZE, (long)body.len);
    curl_easy_setopt(pc->C, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(pc->C, CURLOPT_WRITEDATA, &resp);

    CURLcode rc = curl_easy_perform(pc->C);
    long code = 0;
    if (rc == CURLE_OK)
        curl_easy_getinfo(pc->C, CURLINFO_RESPONSE_CODE, &code);

    free(body.ptr);
    if (rc != CURLE_OK)
    {
        if (resp.ptr)
            free(resp.ptr);
        if (status)
            *status = 0;

        // helpful diagnostics
        const char *eb = pc->errbuf[0] ? pc->errbuf : "(no errbuf)";
        SAP_ERR("curl POST transport error: rc=%d (%s) errbuf=%s\n", (int)rc, curl_easy_strerror(rc), eb);
        return NULL;
    }
    if (status)
        *status = code;
    return resp.ptr;
}

// Authenticated GET (string)
static char *curl_get_auth(const char *url, const char *bearer)
{
    CURL *C = NULL;

    if (!bearer || !*bearer)
    {
        die("GET auth: missing/empty bearer token");
        return NULL;
    }

    C = curl_easy_init();
    if (!C)
    {
        die("curl init");
        return NULL;
    }
    set_common_curl_opts(C);

    struct mem m = {0};
    struct curl_slist *hdr = NULL;
    char auth[1024];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", bearer);
    hdr = curl_slist_append(hdr, auth);
    hdr = curl_slist_append(hdr, "Accept: application/json");
    hdr = curl_slist_append(hdr, "Connection: close");
    curl_easy_setopt(C, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(C, CURLOPT_URL, url);
    curl_easy_setopt(C, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(C, CURLOPT_WRITEDATA, &m);

    CURLcode rc = curl_easy_perform(C);
    if (rc != CURLE_OK)
    {
        const char *msg = curl_easy_strerror(rc);
        curl_slist_free_all(hdr);
        curl_easy_cleanup(C);
        if (m.ptr)
            free(m.ptr);
        die("GET auth: %s", msg);
        return NULL;
    }
    long code = 0;
    curl_easy_getinfo(C, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(hdr);
    curl_easy_cleanup(C);
    if (code / 100 != 2)
    {
        free(m.ptr);
        die("HTTP %ld GET %s", code, url);
        return NULL;
    }
    return m.ptr;
}

static void curl_get_auth_to_file(const char *url, const char *bearer, const char *outpath)
{
    CURL *C = NULL;

    if (!bearer || !*bearer)
    {
        die("GET file: missing/empty bearer token");
        return;
    }

    C = curl_easy_init();
    if (!C)
    {
        die("curl init");
        return;
    }
    set_common_curl_opts(C);

    FILE *f = fopen(outpath, "wb");
    if (!f)
    {
        die("open %s: %s", outpath, strerror(errno));
        curl_easy_cleanup(C);
        return;
    }

    struct curl_slist *hdr = NULL;
    char auth[1024];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", bearer);
    hdr = curl_slist_append(hdr, auth);
    hdr = curl_slist_append(hdr, "Connection: close");
    curl_easy_setopt(C, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(C, CURLOPT_URL, url);
    curl_easy_setopt(C, CURLOPT_WRITEDATA, f);

    CURLcode rc = curl_easy_perform(C);
    if (rc != CURLE_OK)
    {
        curl_slist_free_all(hdr);
        curl_easy_cleanup(C);
        fclose(f);
        die("GET->file: %s", curl_easy_strerror(rc));
        return;
    }
    long code = 0;
    curl_easy_getinfo(C, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(hdr);
    curl_easy_cleanup(C);
    fclose(f);
    if (code / 100 != 2)
        die("HTTP %ld GET %s", code, url);
}

static char *curl_post_multipart_auth(const char *url, const char *bearer,
                                      const char *json_meta, const char *filepath, const char *mime)
{
    CURL *C = curl_easy_init();
    if (!C)
    {
        SAP_ERR("curl init");
        return NULL;
    }
    set_common_curl_opts(C);

    /* For larger uploads, relax no-reuse policy on this handle */
    curl_easy_setopt(C, CURLOPT_FORBID_REUSE, 0L);
    curl_easy_setopt(C, CURLOPT_FRESH_CONNECT, 0L);

    /* Give bigger transfers more time */
    curl_easy_setopt(C, CURLOPT_CONNECTTIMEOUT_MS, 45000L);
    curl_easy_setopt(C, CURLOPT_TIMEOUT_MS, 120000L);

    struct mem m = {0};
    struct curl_slist *hdr = NULL;
    char auth[1024];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", bearer);
    hdr = curl_slist_append(hdr, auth);
    /* IMPORTANT: disable 100-continue */
    hdr = curl_slist_append(hdr, "Expect:");
    /* Do NOT add "Connection: close" here to avoid early FINs mid-upload */

    curl_mime *mp = curl_mime_init(C);
    curl_mimepart *part;

    part = curl_mime_addpart(mp);
    curl_mime_name(part, "metadata");
    curl_mime_data(part, json_meta, CURL_ZERO_TERMINATED);
    curl_mime_type(part, "application/json; charset=UTF-8");

    part = curl_mime_addpart(mp);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, filepath);
    if (mime && *mime)
        curl_mime_type(part, mime);

    curl_easy_setopt(C, CURLOPT_URL, url);
    curl_easy_setopt(C, CURLOPT_MIMEPOST, mp);
    curl_easy_setopt(C, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(C, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(C, CURLOPT_WRITEDATA, &m);

    CURLcode rc = curl_easy_perform(C);
    long code = 0;
    if (rc == CURLE_OK)
        curl_easy_getinfo(C, CURLINFO_RESPONSE_CODE, &code);

    curl_mime_free(mp);
    curl_slist_free_all(hdr);
    curl_easy_cleanup(C);

    if (rc != CURLE_OK)
    {
        SAP_ERR("POST multipart: %s\n", curl_easy_strerror(rc));
        if (m.ptr)
            free(m.ptr);
        return NULL;
    }
    if (code / 100 != 2)
    {
        if (m.ptr)
            free(m.ptr);
        SAP_ERR("HTTP %ld POST multipart %s\n", code, url);
        return NULL;
    }
    return m.ptr;
}

// ---- Tokens
typedef struct
{
    char *access_token;
    char *refresh_token;
    char *scope;
    char *token_type;
    time_t expires_at;
    char *client_id;
    char *client_secret;
} tokens_t;

static void tokens_free(tokens_t *t)
{
    if (!t)
        return;
    free(t->access_token);
    free(t->refresh_token);
    free(t->scope);
    free(t->token_type);
    free(t->client_id);
    free(t->client_secret);
    memset(t, 0, sizeof(*t));
}

static char tokens_json_buf[4 * PAGE_SIZE_4K] = {0};

static int tokens_load(tokens_t *t)
{
    if (!t)
        return -1;

    /* Empty buffer -> nothing saved yet */
    if (tokens_json_buf[0] == '\0')
        return -1;

    json_error_t e;
    /* Use loads: your buffer is NUL-terminated */
    json_t *J = json_loads(tokens_json_buf, 0, &e);
    if (!J)
        return -1;

    memset(t, 0, sizeof(*t));

    json_t *v;
#define GETS(name)                                \
    do                                            \
    {                                             \
        v = json_object_get(J, #name);            \
        if (v && json_is_string(v))               \
            t->name = sdup(json_string_value(v)); \
    } while (0)

    GETS(access_token);
    GETS(refresh_token);
    GETS(scope);
    GETS(token_type);
    GETS(client_id);
    GETS(client_secret);
#undef GETS

    v = json_object_get(J, "expires_at");
    if (v && json_is_integer(v))
        t->expires_at = (time_t)json_integer_value(v);

    json_decref(J);
    return 0;
}

static int tokens_save(const tokens_t *t)
{
    if (!t)
        return -1;

    int rc = 0;
    json_t *J = json_object();
    if (!J)
        return -1;

#define PUTS(name) json_object_set_new(J, #name, json_string(t->name ? t->name : ""))
    PUTS(access_token);
    PUTS(refresh_token);
    PUTS(scope);
    PUTS(token_type);
    PUTS(client_id);
    PUTS(client_secret);
#undef PUTS

    json_object_set_new(J, "expires_at", json_integer((json_int_t)t->expires_at));

    char *json_string = json_dumps(J, JSON_INDENT(2));
    if (!json_string)
    {
        rc = 1;
        goto out;
    }

    size_t need = strlen(json_string) + 1; /* include NUL */
    if (need > sizeof(tokens_json_buf))
    {
        rc = 2; /* ENOSPC */
        goto out_free;
    }

    /* Safe copy; ensures NUL-termination */
    memcpy(tokens_json_buf, json_string, need);
    rc = 0;

out_free:
    free(json_string);
out:
    json_decref(J);
    return rc;
}

static char *oa2_post_form(const char *url, const char *pairs[][2], size_t npairs)
{
    // simple one-shot POST for refresh; returns body only if 2xx
    post_client_t pc;
    post_client_init(&pc);
    long st = 0;
    char *r = post_client_perform(&pc, url, pairs, npairs, &st);
    post_client_cleanup(&pc);
    if (!r)
    {
        die("refresh: transport error");
        return NULL;
    }
    if (st / 100 != 2)
    {
        free(r);
        die("HTTP %ld POST %s", st, url);
        return NULL;
    }
    return r;
}

static void refresh_access_token(tokens_t *T)
{
    const char *pairs_no_secret[][2] = {
        {"client_id", T->client_id},
        {"grant_type", "refresh_token"},
        {"refresh_token", T->refresh_token}};
    const char *pairs_with_secret[][2] = {
        {"client_id", T->client_id},
        {"client_secret", T->client_secret ? T->client_secret : ""},
        {"grant_type", "refresh_token"},
        {"refresh_token", T->refresh_token}};
    size_t n = (T->client_secret && *T->client_secret) ? 4 : 3;
    const char *(*pp)[2] = (n == 4) ? pairs_with_secret : pairs_no_secret;

    char *resp = oa2_post_form(TOKEN_URL, pp, n);
    if (!resp)
        die("refresh: no response");

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
        die("refresh parse: %s", je.text);

    const char *at = json_string_value(json_object_get(J, "access_token"));
    const char *tt = json_string_value(json_object_get(J, "token_type"));
    const char *sc = json_string_value(json_object_get(J, "scope"));
    int expires_in = (int)json_integer_value(json_object_get(J, "expires_in"));
    const char *rt = json_string_value(json_object_get(J, "refresh_token"));

    if (!at || !tt)
    {
        json_decref(J);
        die("refresh missing fields");
    }
    free(T->access_token);
    T->access_token = sdup(at);
    free(T->token_type);
    T->token_type = sdup(tt);
    if (sc)
    {
        free(T->scope);
        T->scope = sdup(sc);
    }
    if (rt && *rt)
    {
        free(T->refresh_token);
        T->refresh_token = sdup(rt);
    }
    T->expires_at = time(NULL) + (expires_in ? expires_in : 3600);
    json_decref(J);
    tokens_save(T);
}

static void ensure_fresh_access_token(tokens_t *T)
{
    if (tokens_load(T) != 0)
        die("Not logged in. Run: gdrive login ...");
    time_t now = time(NULL);
    if (T->access_token && (now + 60) < T->expires_at)
        return;
    if (!T->refresh_token || !*T->refresh_token)
        die("No refresh_token; re-login needed.");
    refresh_access_token(T);
}

static void device_user_code_free(device_user_code_t *d)
{
    if (!d)
        return;

    if (d->device_code)
    {
        free(d->device_code);
        d->device_code = NULL;
    }

    if (d->user_code)
    {
        free(d->user_code);
        d->user_code = NULL;
    }

    if (d->verification_uri)
    {
        free(d->verification_uri);
        d->verification_uri = NULL;
    }

    if (d->verification_uri_with_code)
    {
        free(d->verification_uri_with_code);
        d->verification_uri_with_code = NULL;
    }

    if (d->verification_uri_complete)
    {
        free(d->verification_uri_complete);
        d->verification_uri_complete = NULL;
    }

    memset(d, 0, sizeof(*d));
}

// -------------------------------------------------------------------------------------
// Step 1: Request device & user code
static SAP_STATUS _gdrive_login_query_device_user_code(const char *client_id,
                                                       const char *scope,
                                                       device_user_code_t *out)
{
    if (!out)
        return EINVAL;
    memset(out, 0, sizeof(*out));

    post_client_t pc_dc;
    post_client_init(&pc_dc);
    curl_easy_setopt(pc_dc.C, CURLOPT_URL, DEVICE_CODE_URL);

    const char *pairs_dc[][2] = {
        {"client_id", client_id},
        {"scope", scope}};

    long st = 0;
    char *resp = post_client_perform(&pc_dc, DEVICE_CODE_URL, pairs_dc, 2, &st);
    post_client_cleanup(&pc_dc);

    if (!resp)
    {
        sleep_seconds(2); // one retry
        post_client_t pc2;
        post_client_init(&pc2);
        long st2 = 0;
        resp = post_client_perform(&pc2, DEVICE_CODE_URL, pairs_dc, 2, &st2);
        post_client_cleanup(&pc2);
        if (!resp)
        {
            SAP_ERR("device_code: transport error!\n");
            return EINTERNAL;
        }
        st = st2;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        SAP_ERR("device_code parse: %s!\n", je.text);
        return EINTERNAL;
    }

    if (st / 100 != 2)
    {
        const char *es = json_string_value(json_object_get(J, "error"));
        const char *ed = json_string_value(json_object_get(J, "error_description"));
        SAP_ERR("device_code error (%ld): %s%s%s\n", st, es ? es : "(unknown)",
                (ed && *ed) ? " - " : "", (ed && *ed) ? ed : "");
        json_decref(J);
        return EINTERNAL;
    }

    const char *device_code = json_string_value(json_object_get(J, "device_code"));
    const char *user_code = json_string_value(json_object_get(J, "user_code"));
    const char *v_uri = json_string_value(json_object_get(J, "verification_uri"));
    if (!v_uri)
        v_uri = json_string_value(json_object_get(J, "verification_url"));
    const char *v_complete = json_string_value(json_object_get(J, "verification_uri_complete"));
    int interval = (int)json_integer_value(json_object_get(J, "interval"));
    int expires_in = (int)json_integer_value(json_object_get(J, "expires_in"));
    if (!interval)
        interval = min_poll_interval;

    if (!device_code || !user_code || !v_uri)
    {
        json_decref(J);
        SAP_ERR("device_code: missing fields!\n");
        return EINTERNAL;
    }

    // Duplicate with checks
    out->device_code = sdup(device_code);
    out->user_code = sdup(user_code);
    out->verification_uri = sdup(v_uri);
    out->verification_uri_complete = (v_complete && *v_complete) ? sdup(v_complete) : NULL;
    out->interval = interval;
    out->expires_in = expires_in;

    if (!out->device_code || !out->user_code || !out->verification_uri ||
        ((v_complete && *v_complete) && !out->verification_uri_complete))
    {
        device_user_code_free(out);
        json_decref(J);
        SAP_ERR("device_code: OOM duplicating fields!\n");
        return EINTERNAL;
    }

    // Guidance
    char quick[2048] = {0};
    if (out->verification_uri_complete && *out->verification_uri_complete)
        snprintf(quick, sizeof(quick), "%s", out->verification_uri_complete);
    else
        snprintf(quick, sizeof(quick), "%s?user_code=%s", out->verification_uri, out->user_code);

    out->verification_uri_with_code = sdup(quick);

    SAP_INFO("=== Scan or open URL ===\n%s\n", out->verification_uri_with_code);
    SAP_INFO("=== If typing manually, visit ===\n%s\n", out->verification_uri);
    SAP_INFO("=== Enter code ===\n%s\n\n", out->user_code);

    json_decref(J);
    return SUCCESS;
}

/// @brief Init the internal state of polling GDrive login tokens.
/// @param S
/// @param duc
static void _gdrive_login_tokens_poll_state_init(gdrive_tokens_poll_state_t *S, const device_user_code_t *duc)
{
    memset(S, 0, sizeof(*S));
    post_client_init(&S->pc);
    S->pc_inited = true;
    curl_easy_setopt(S->pc.C, CURLOPT_URL, TOKEN_URL);

    S->start = time(NULL);
    S->poll_interval = max(duc->interval, min_poll_interval);

    // first attempt is immediate
    S->next_poll_at = S->start;
}

/// @brief Finalize the internal state of polling GDrive login tokens.
/// @param S
static void _gdrive_login_tokens_poll_state_fini(gdrive_tokens_poll_state_t *S)
{
    if (S->pc_inited)
    {
        post_client_cleanup(&S->pc);
        S->pc_inited = false;
    }
}

// Returns:
//  - SUCCESS: success and tokens.json saved (DONE).
//  - EAGAIN : not time yet OR retry scheduled for the future (NOT DONE).
//  - EINTERNAL / other FAIL code: hard failure (STOP).
static SAP_STATUS _gdrive_login_poll_once(const char *client_id,
                                          const char *client_secret,
                                          const char *initial_scope,
                                          const device_user_code_t *duc,
                                          gdrive_tokens_poll_state_t *S)
{
    if (!client_id || !duc || !duc->device_code || !S)
        return EINVAL;

    // If it's not time yet, return immediately (no wait).
    time_t now = time(NULL);
    if (now < S->next_poll_at)
        return EAGAIN;

    // Check overall device_code lifetime before doing work.
    if (duc->expires_in && (now - S->start) > duc->expires_in)
    {
        SAP_ERR("authorization timed out!\n");
        return EINTERNAL;
    }

    // Prepare POST form pairs
    const char *cid = client_id;
    const char *csec = client_secret ? client_secret : "";

    const char *pairs2_no_secret[][2] = {
        {"client_id", cid},
        {"device_code", duc->device_code},
        {"grant_type", "urn:ietf:params:oauth:grant-type:device_code"},
    };
    const char *pairs2_with_secret[][2] = {
        {"client_id", cid},
        {"client_secret", csec},
        {"device_code", duc->device_code},
        {"grant_type", "urn:ietf:params:oauth:grant-type:device_code"},
    };
    size_t n = (csec && *csec) ? 4 : 3;
    const char *(*pp)[2] = (n == 4) ? pairs2_with_secret : pairs2_no_secret;

    long pst = 0;
    char *resp2 = post_client_perform(&S->pc, TOKEN_URL, pp, n, &pst);
    if (!resp2)
    {
        // Transport failure path
        S->consec_transport_err++;
        if (S->first_transport_fail == 0)
            S->first_transport_fail = time(NULL);

        // Too-long wall failure?
        if ((time(NULL) - S->first_transport_fail) > MAX_POLL_WALL_FAIL_SEC)
        {
            SAP_ERR("network unstable for too long; please re-run login.\n");
            return EINTERNAL;
        }
        // Overall timeout?
        if (duc->expires_in && (time(NULL) - S->start) > duc->expires_in)
        {
            SAP_ERR("authorization timed out (transport)!\n");
            return EINTERNAL;
        }
        // Recreate handle after repeated transient transport errors
        if (S->consec_transport_err >= MAX_CONSEC_TRANSIENT_ERR)
        {
            post_client_cleanup(&S->pc);
            post_client_init(&S->pc);
            curl_easy_setopt(S->pc.C, CURLOPT_URL, TOKEN_URL);
            S->consec_transport_err = 0;
        }

        // schedule next retry with jitter; tell caller "come back later"
        int jitter = 1 + (rand() % 3);
        S->next_poll_at = time(NULL) + S->poll_interval + jitter;
        return EAGAIN;
    }

    // Reset transport-error tracking on any HTTP response
    S->consec_transport_err = 0;
    S->first_transport_fail = 0;

    // Transient HTTP statuses → schedule retry; return EAGAIN
    if (pst == 428 || pst == 409 || pst == 408 || pst == 429 || (pst / 100) == 5)
    {
        free(resp2);
        S->next_poll_at = time(NULL) + S->poll_interval;
        return EAGAIN;
    }

    // Parse JSON
    json_error_t je2;
    json_t *K = json_loads(resp2, 0, &je2);
    free(resp2);
    if (!K)
    {
        SAP_ERR("token poll parse: %s!\n", je2.text);
        return EINTERNAL;
    }

    // Non-2xx with known soft errors
    if (pst / 100 != 2)
    {
        const char *es = json_string_value(json_object_get(K, "error"));
        const char *ed = json_string_value(json_object_get(K, "error_description"));

        if (es && !strcmp(es, "authorization_pending"))
        {
            json_decref(K);
            S->next_poll_at = time(NULL) + S->poll_interval;
            return EAGAIN;
        }
        if (es && !strcmp(es, "slow_down"))
        {
            json_decref(K);
            S->poll_interval += 5;
            S->next_poll_at = time(NULL) + S->poll_interval;
            return EAGAIN;
        }

        SAP_ERR("token poll error (%ld): %s%s%s\n", pst, es ? es : "(unknown)",
                (ed && *ed) ? " - " : "", (ed && *ed) ? ed : "");
        json_decref(K);
        return EINTERNAL;
    }

    // Success path
    const char *at = json_string_value(json_object_get(K, "access_token"));
    const char *rt = json_string_value(json_object_get(K, "refresh_token"));
    const char *tt = json_string_value(json_object_get(K, "token_type"));
    int at_expires = (int)json_integer_value(json_object_get(K, "expires_in"));
    const char *sc = json_string_value(json_object_get(K, "scope"));
    if (!at || !tt)
    {
        json_decref(K);
        SAP_ERR("token poll: missing fields!\n");
        return EINTERNAL;
    }

    // Build tokens blob
    tokens_t T = (tokens_t){0};
    T.client_id = sdup(client_id);
    T.client_secret = sdup(client_secret ? client_secret : "");
    T.scope = sdup(initial_scope ? initial_scope : "");
    T.access_token = sdup(at);
    T.token_type = sdup(tt);
    if (!T.client_id || !T.client_secret || !T.scope || !T.access_token || !T.token_type)
    {
        json_decref(K);
        tokens_free(&T);
        SAP_ERR("OOM duplicating token fields\n");
        return ENOMEM;
    }
    if (rt)
    {
        T.refresh_token = sdup(rt);
        if (!T.refresh_token)
        {
            json_decref(K);
            tokens_free(&T);
            SAP_ERR("OOM duplicating refresh_token\n");
            return ENOMEM;
        }
    }
    if (sc)
    {
        char *new_scope = sdup(sc);
        if (!new_scope)
        {
            json_decref(K);
            tokens_free(&T);
            SAP_ERR("OOM duplicating scope\n");
            return ENOMEM;
        }
        free(T.scope);
        T.scope = new_scope;
    }
    T.expires_at = time(NULL) + (at_expires ? at_expires : 3600);

    json_decref(K);

    if (tokens_save(&T) != 0)
    {
        tokens_free(&T);
        SAP_ERR("Save access tokens failed!\n");
        return EINTERNAL;
    }
    SAP_INFO("Login OK. Tokens saved.\n");
    tokens_free(&T);

    // Success; no need to schedule next poll
    return SUCCESS;
}

// Wait helper (unchanged except 1ms yields).
static void _wait_until(time_t deadline)
{
    for (;;)
    {
        time_t now = time(NULL);
        if (now >= deadline)
            break;
        usleep(1000);
    }
}

// Blocking variant built on poll-once
static SAP_STATUS _gdrive_login_poll_tokens_busyloop(const char *client_id,
                                                     const char *client_secret,
                                                     const char *initial_scope,
                                                     const device_user_code_t *duc)
{
    if (!client_id || !duc || !duc->device_code)
        return EINVAL;

    gdrive_tokens_poll_state_t S;
    _gdrive_login_tokens_poll_state_init(&S, duc);

    SAP_STATUS rc = SUCCESS;
    for (;;)
    {
        // Pump UI/events while waiting for the next allowed attempt
        _wait_until(S.next_poll_at);

        rc = _gdrive_login_poll_once(client_id, client_secret, initial_scope, duc, &S);
        if (rc == SUCCESS)
        {
            // done
            break;
        }
        if (rc == EAGAIN)
        {
            // not yet / retry scheduled — keep looping
            continue;
        }
        // Hard failure
        break;
    }

    _gdrive_login_tokens_poll_state_fini(&S);
    return rc;
}

/// @brief Blocking function for GDrive login, including (1) getting device code and user code, and (2) poll for login
/// tokens.
static SAP_STATUS _gdrive_login_block(const char *client_id, const char *client_secret, const char *scope)
{
    SAP_STATUS status = SUCCESS;
    device_user_code_t duc;

    status = _gdrive_login_query_device_user_code(client_id, scope, &duc);
    if (FAIL(status))
    {
        goto out;
    }

    status = _gdrive_login_poll_tokens_busyloop(client_id, client_secret, scope, &duc);
    if (FAIL(status))
    {
        goto out;
    }

    // On success
    status = SUCCESS;

out:
    device_user_code_free(&duc);
    return status;
}

int64_t _fs_local_get_file_size(const char *filename)
{
    if (!filename || !*filename)
    {
        errno = EINVAL;
        return -1;
    }

    struct stat st;
    if (stat(filename, &st) == -1)
    {
        // errno already set by stat()
        return -1;
    }

    // For non-regular files, st_size may be non-meaningful (e.g., directories, pipes).
    // If you only want regular files, uncomment the check below:
    // if (!S_ISREG(st.st_mode)) { errno = EISDIR; return -1; }

    return (int64_t)st.st_size;
}

// ---- Drive helpers
static char *_drive_upload_file(const char *bearer, const char *local_path, const char *name_opt,
                                const char *parent_id_opt, const char *mime_opt)
{
    int64_t file_size = 0;
    (void)parent_id_opt;
    const char *base = name_opt ? name_opt : basename_x(local_path);

    // Check: file size must be within the limit.
    file_size = _fs_local_get_file_size(local_path);
    if (file_size < 0)
    {
        SAP_ERR("GDrive Upload: File (%s) has invalid size! (size: %lld) \n", local_path, (long long)file_size);
        return NULL;
    }
    else if (file_size > GDRIVE_UPLOAD_DOWNLOAD_FILE_SIZE_MAX)
    {
        SAP_ERR("GDrive Upload: File (%s) is too big! (size: %lld) \n", local_path, (long long)file_size);
        return NULL;
    }

    json_t *meta = json_object();
    json_object_set_new(meta, "name", json_string(base));
    if (parent_id_opt && *parent_id_opt)
    {
        json_t *arr = json_array();
        json_array_append_new(arr, json_string(parent_id_opt));
        json_object_set_new(meta, "parents", arr);
    }
    char *meta_str = json_dumps(meta, JSON_COMPACT);
    json_decref(meta);

    char *resp = curl_post_multipart_auth(DRIVE_UPLOAD, bearer, meta_str, local_path, mime_opt);
    free(meta_str);

    if (!resp)
    {
        // Upload failed; curl_post_multipart_auth already logged details
        return NULL;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        die("upload parse: %s", je.text);
        return NULL;
    }

    const char *id = json_string_value(json_object_get(J, "id"));
    if (!id)
    {
        json_decref(J);
        die("upload: no id");
        return NULL;
    }

    char *ret = sdup(id);
    json_decref(J);

    return ret;
}

static char *drive_create_folder(const char *bearer, const char *name, const char *parent_id_opt)
{
    char *ret = NULL;
    char *url = NULL;
    json_t *J = NULL;

    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        goto out;
    }

    snprintf(url, URL_MAX_SIZE, "%s/files", DRIVE_API);
    CURL *C = curl_easy_init();
    if (!C)
    {
        die("curl init");
        goto out;
    }
    set_common_curl_opts(C);

    struct mem m = {0};
    struct curl_slist *hdr = NULL;
    char auth[1024];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", bearer);
    hdr = curl_slist_append(hdr, auth);
    hdr = curl_slist_append(hdr, "Content-Type: application/json; charset=UTF-8");
    hdr = curl_slist_append(hdr, "Accept: application/json");
    hdr = curl_slist_append(hdr, "Connection: close");

    json_t *meta = json_object();
    json_object_set_new(meta, "name", json_string(name));
    json_object_set_new(meta, "mimeType", json_string("application/vnd.google-apps.folder"));
    if (parent_id_opt && *parent_id_opt)
    {
        json_t *arr = json_array();
        json_array_append_new(arr, json_string(parent_id_opt));
        json_object_set_new(meta, "parents", arr);
    }
    char *meta_str = json_dumps(meta, JSON_COMPACT);
    json_decref(meta);

    curl_easy_setopt(C, CURLOPT_URL, url);
    curl_easy_setopt(C, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(C, CURLOPT_POST, 1L);
    curl_easy_setopt(C, CURLOPT_POSTFIELDS, meta_str);
    curl_easy_setopt(C, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(C, CURLOPT_WRITEDATA, &m);

    CURLcode rc = curl_easy_perform(C);
    curl_check(rc, "folder create");
    long code = 0;
    curl_easy_getinfo(C, CURLINFO_RESPONSE_CODE, &code);
    curl_slist_free_all(hdr);
    curl_easy_cleanup(C);
    free(meta_str);
    if (code / 100 != 2)
    {
        free(m.ptr);
        die("HTTP %ld creating folder", code);
        goto out;
    }

    json_error_t je;
    J = json_loads(m.ptr, 0, &je);
    free(m.ptr);
    if (!J)
    {
        die("create folder parse: %s", je.text);
        goto out;
    }
    const char *id = json_string_value(json_object_get(J, "id"));
    if (!id)
    {
        json_decref(J);
        die("create folder: no id");
        goto out;
    }

    ret = sdup(id);

out:
    if (J)
        json_decref(J);

    if (url)
        url_free(url);

    return ret;
}

char *drive_find_or_create_path(const char *bearer, const char *parent_id, const char *comp)
{
    char *ret = NULL;
    char *url = NULL;

    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        goto err;
    }

    CURL *C = curl_easy_init();
    if (!C)
    {
        die("curl init");
        goto err;
    }
    set_common_curl_opts(C);

    char q_raw[1024];
    snprintf(q_raw, sizeof(q_raw),
             "name='%s' and mimeType='application/vnd.google-apps.folder' and '%s' in parents and trashed=false",
             comp, parent_id);
    char *q_esc = escape_str(C, q_raw);

    snprintf(url, URL_MAX_SIZE, "%s/files?pageSize=10&fields=files(id,name)&q=%s", DRIVE_API, q_esc);

    curl_easy_cleanup(C);
    free(q_esc);

    char *resp = curl_get_auth(url, bearer);
    if (!resp)
    {
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        goto err;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        die("find folder parse: %s", je.text);
        goto err;
    }
    json_t *files = json_object_get(J, "files");
    json_t *first = (files && json_array_size(files) > 0) ? json_array_get(files, 0) : NULL;
    const char *id = first ? json_string_value(json_object_get(first, "id")) : NULL;
    if (id)
        ret = sdup(id);

    json_decref(J);
    if (url)
        url_free(url);

    if (ret)
        return ret;
    else
        return drive_create_folder(bearer, comp, parent_id);

err:
    if (url)
        url_free(url);

    return NULL;
}

static int drive_list(const char *bearer, const char *parent_opt)
{
    CURL *C = curl_easy_init();
    if (!C)
    {
        die("curl init");
        return 1;
    }
    set_common_curl_opts(C);

    char q_raw[1024] = {0};
    if (parent_opt && *parent_opt)
        snprintf(q_raw, sizeof(q_raw), "trashed=false and '%s' in parents", parent_opt);
    else
        snprintf(q_raw, sizeof(q_raw), "trashed=false");

    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        curl_easy_cleanup(C);
        return 1;
    }

    char *q_esc = escape_str(C, q_raw);
    snprintf(url, URL_MAX_SIZE,
             "%s/files?pageSize=1000"
             "&fields=nextPageToken,files(id,name,mimeType,parents)"
             "&q=%s"
             "&supportsAllDrives=true&includeItemsFromAllDrives=true",
             DRIVE_API, q_esc);

    curl_easy_cleanup(C);
    free(q_esc);

    char *resp = curl_get_auth(url, bearer);
    url_free(url);
    if (!resp)
    {
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        return 1;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        SAP_ERR("list parse: %s!\n", je.text);
        return 1;
    }

    json_t *files = json_object_get(J, "files");
    if (!files || !json_is_array(files))
    {
        json_decref(J);
        SAP_ERR("list: files[] !\n");
        return 1;
    }
    size_t i;
    json_t *it;
    json_array_foreach(files, i, it)
    {
        const char *id = json_string_value(json_object_get(it, "id"));
        const char *nm = json_string_value(json_object_get(it, "name"));
        const char *mt = json_string_value(json_object_get(it, "mimeType"));
        SAP_INFO("%-40s  %-4s  %s\n", id, (mt && strstr(mt, "folder")) ? "DIR" : "FILE", nm ? nm : "(no name)");
    }
    json_decref(J);
    return 0;
}

static void drive_download_file(const char *bearer, const char *id, const char *name, const char *dest)
{
    char path[1024] = {0};
    path_join(path, sizeof(path), dest, name);

    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        return;
    }

    snprintf(url, URL_MAX_SIZE, "%s/files/%s?alt=media", DRIVE_API, id);
    curl_get_auth_to_file(url, bearer, path);
    url_free(url);

    SAP_INFO("saved: %s\n", path);
}

static void drive_download_folder(const char *bearer, const char *id, const char *name, const char *dest)
{
    CURL *C = curl_easy_init();
    if (!C)
    {
        die("curl init");
        return;
    }
    set_common_curl_opts(C);

    char newdir[1024];
    path_join(newdir, sizeof(newdir), dest, name);
    if (ensure_dir(newdir) != 0)
    {
        curl_easy_cleanup(C);
        die("mkdir %s failed", newdir);
    }

    char q_raw[1024];
    snprintf(q_raw, sizeof(q_raw), "'%s' in parents and trashed=false", id);

    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        curl_easy_cleanup(C);
        return;
    }

    char *q_esc = escape_str(C, q_raw);

    snprintf(url, URL_MAX_SIZE, "%s/files?pageSize=1000&fields=files(id,name,mimeType)&q=%s", DRIVE_API, q_esc);

    curl_easy_cleanup(C);
    free(q_esc);

    char *resp = curl_get_auth(url, bearer);
    url_free(url);
    if (!resp)
    {
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        return;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
        die("children parse: %s", je.text);

    size_t i;
    json_t *it;
    json_t *files = json_object_get(J, "files");
    json_array_foreach(files, i, it)
    {
        const char *cid = json_string_value(json_object_get(it, "id"));
        const char *nm = json_string_value(json_object_get(it, "name"));
        const char *mt = json_string_value(json_object_get(it, "mimeType"));
        if (mt && strstr(mt, "folder"))
            drive_download_folder(bearer, cid, nm, newdir);
        else
            drive_download_file(bearer, cid, nm, newdir);
    }
    json_decref(J);
}

__attribute__((unused)) static void drive_download_recursive(const char *bearer, const char *id, const char *dest)
{
    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        return;
    }

    snprintf(url, URL_MAX_SIZE, "%s/files/%s?fields=id,name,mimeType", DRIVE_API, id);
    char *resp = curl_get_auth(url, bearer);
    url_free(url);
    if (!resp)
    {
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        return;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        die("meta parse: %s", je.text);
        return;
    }

    const char *name = json_string_value(json_object_get(J, "name"));
    const char *mt = json_string_value(json_object_get(J, "mimeType"));

    if (!name || !mt)
    {
        json_decref(J);
        die("missing name/mimeType");
        return;
    }

    if (strstr(mt, "folder"))
        drive_download_folder(bearer, id, name, dest);
    else
        drive_download_file(bearer, id, name, dest);
    json_decref(J);
}

static SAP_STATUS _gdrive_list_rootdir(void)
{
    tokens_t T = {0};
    ensure_fresh_access_token(&T);

    if (!T.access_token || !*T.access_token)
    {
        SAP_ERR("_gdrive_list_rootdir: no access token (login/refresh failed)\n");
        tokens_free(&T);
        return EINTERNAL;
    }

    int ret = drive_list(T.access_token, "root");
    tokens_free(&T);

    return ret ? EINTERNAL : SUCCESS;
}

static char *_gdrive_upload_file(const char *local_path, const char *name_opt,
                                 const char *parent_id_opt, const char *mime_opt)
{
    tokens_t T = {0};
    ensure_fresh_access_token(&T);

    if (!T.access_token || !*T.access_token)
    {
        SAP_ERR("_gdrive_upload_file: no access token (login/refresh failed)\n");
        tokens_free(&T);
        return NULL;
    }

    char *ret = _drive_upload_file(T.access_token, local_path, name_opt, parent_id_opt, mime_opt);

    tokens_free(&T);

    return ret;
}

/********* Public functions *********/
SAP_STATUS gdrive_init(SAP_CONFIG *sap_cfg)
{
    SAP_STATUS status = SUCCESS;

    // Check: params must be valid
    if (!sap_cfg)
        return EINVAL;

    status = sslca_init(sap_cfg->fs_local_info.certs_filepath);
    if (FAIL(status))
    {
        SAP_ERR("GDrive Init Error: SSL CA init error! status:%lu\n", status);
        return EINTERNAL;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create a shared object to share DNS cache across all easy handles
    g_share = curl_share_init();
    if (g_share)
    {
        curl_share_setopt(g_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        // single-threaded: no lock callbacks required
    }

    // On success
    return SUCCESS;
}

void gdrive_fini(void)
{
    if (g_share)
    {
        curl_share_cleanup(g_share);
        g_share = NULL;
    }

    sslca_fini();
}

SAP_STATUS gdrive_login_query_device_user_code(device_user_code_t *out_duc)
{
    return _gdrive_login_query_device_user_code(g_client_id, g_scope, out_duc);
}

SAP_STATUS gdrive_login_poll_tokens(const device_user_code_t *duc)
{
    return _gdrive_login_poll_tokens_busyloop(g_client_id, g_client_secret, g_scope, duc);
}

gdrive_tokens_poll_state_t *gdrive_login_tokens_poll_state_init(const device_user_code_t *duc)
{
    gdrive_tokens_poll_state_t *S = NULL;

    S = malloc(sizeof(gdrive_tokens_poll_state_t));
    if (!S)
        return NULL;

    _gdrive_login_tokens_poll_state_init(S, duc);
    return S;
}

void gdrive_login_tokens_poll_state_fini(gdrive_tokens_poll_state_t *S)
{
    if (S)
    {
        _gdrive_login_tokens_poll_state_fini(S);
        free(S);
    }
}

SAP_STATUS gdrive_login_poll_tokens_once(const device_user_code_t *duc, gdrive_tokens_poll_state_t *S)
{
    return _gdrive_login_poll_once(g_client_id, g_client_secret, g_scope, duc, S);
}

SAP_STATUS gdrive_login(char *username, char *password)
{
    (void)username;
    (void)password;

    int ret = _gdrive_login_block(g_client_id, g_client_secret, g_scope);
    if (ret)
    {
        SAP_ERR("login failed\n");
        return EINTERNAL;
    }
    return SUCCESS;
}

/* Trim RFC3339 "2024-09-29T13:11:22.123Z" -> "2024-09-29 13:11" for UI */
static char *trim_time_for_ui(const char *rfc3339)
{
    if (!rfc3339)
        return sdup("");
    char buf[32] = {0};
    if (strlen(rfc3339) >= 16)
    {
        /* yyyy-mm-dd + space + hh:mm */
        snprintf(buf, sizeof(buf), "%.10s %.5s", rfc3339, rfc3339 + 11);
        return sdup(buf);
    }
    return sdup(rfc3339);
}

static unsigned long long parse_ull(const char *s)
{
    if (!s || !*s)
        return 0ULL;
    unsigned long long v = 0ULL;
    /* json "size" is a string; robust parsing */
    sscanf(s, "%llu", &v);
    return v;
}

int gdrive_list_children(const char *parent_id, gdrive_entry_t **out, size_t *out_n)
{
    if (!out || !out_n)
        return 1;
    *out = NULL;
    *out_n = 0;

    tokens_t T = {0};
    ensure_fresh_access_token(&T);
    if (!T.access_token || !*T.access_token)
    {
        tokens_free(&T);
        return 1;
    }

    const char *pid = (!parent_id || !*parent_id) ? "root" : (!strcmp(parent_id, "..") ? "root" : parent_id);

    CURL *C = curl_easy_init();
    if (!C)
    {
        tokens_free(&T);
        return 1;
    }
    set_common_curl_opts(C);

    char q_raw[1024];
    snprintf(q_raw, sizeof(q_raw), "trashed=false and '%s' in parents", pid);

    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        curl_easy_cleanup(C);
        tokens_free(&T);
        return 1;
    }

    char *q_esc = escape_str(C, q_raw);

    snprintf(url, URL_MAX_SIZE,
             "%s/files?pageSize=1000"
             "&fields=files(id,name,mimeType,modifiedTime,size)"
             "&q=%s"
             "&supportsAllDrives=true&includeItemsFromAllDrives=true",
             DRIVE_API, q_esc);

    curl_easy_cleanup(C);
    free(q_esc);

    char *resp = curl_get_auth(url, T.access_token);
    url_free(url);
    tokens_free(&T);
    if (!resp)
    {
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        return 1;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        SAP_ERR("list parse: %s!\n", je.text);
        return 1;
    }

    json_t *files = json_object_get(J, "files");
    if (!files || !json_is_array(files))
    {
        json_decref(J);
        return 1;
    }

    size_t n = json_array_size(files);
    gdrive_entry_t *arr = (gdrive_entry_t *)calloc(n, sizeof(gdrive_entry_t));
    if (!arr)
    {
        json_decref(J);
        return 1;
    }

    for (size_t i = 0; i < n; i++)
    {
        json_t *it = json_array_get(files, i);
        const char *id = json_string_value(json_object_get(it, "id"));
        const char *nm = json_string_value(json_object_get(it, "name"));
        const char *mt = json_string_value(json_object_get(it, "mimeType"));
        const char *mtm = json_string_value(json_object_get(it, "modifiedTime"));
        const char *szs = json_string_value(json_object_get(it, "size"));

        arr[i].id = sdup(id);
        arr[i].name = sdup(nm);
        arr[i].is_folder = (mt && strstr(mt, "folder")) ? 1 : 0;
        arr[i].modified_time = trim_time_for_ui(mtm);
        arr[i].size = arr[i].is_folder ? 0ULL : parse_ull(szs);
    }
    json_decref(J);

    *out = arr;
    *out_n = n;
    return 0;
}

void gdrive_entries_free(gdrive_entry_t *arr, size_t n)
{
    (void)n;
    if (!arr)
        return;
    for (size_t i = 0;; i++)
    {
        /* We don't know n at call sites sometimes; but caller passes it.
           We'll still loop n times to be explicit. */
        if (i >= n)
            break;
        free(arr[i].id);
        free(arr[i].name);
        free(arr[i].modified_time);
    }
    free(arr);
}

int gdrive_download_to(const char *id, const char *local_dir)
{
    int ret = 0;
    tokens_t T = {0};

    if (!id || !local_dir)
        return 1;

    ensure_fresh_access_token(&T);
    if (!T.access_token || !*T.access_token)
    {
        tokens_free(&T);
        return 1;
    }

    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        tokens_free(&T);
        return 1;
    }

    snprintf(url, URL_MAX_SIZE, "%s/files/%s?fields=id,name,mimeType,size", DRIVE_API, id);
    char *resp = curl_get_auth(url, T.access_token);
    url_free(url);
    if (!resp)
    {
        tokens_free(&T);
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        return 1;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
    {
        tokens_free(&T);
        return 1;
    }

    const char *name_s = json_string_value(json_object_get(J, "name"));
    const char *mt = json_string_value(json_object_get(J, "mimeType"));
    const char *szs = json_string_value(json_object_get(J, "size"));
    int is_folder = (mt && strstr(mt, "folder")) ? 1 : 0;

    /* Make safe copies / values BEFORE decref */
    char *name = name_s ? sdup(name_s) : sdup("(unnamed)");
    unsigned long long rfile_size = szs ? parse_ull(szs) : 0ULL;

    json_decref(J);

    if (!name)
    {
        tokens_free(&T);
        return 1;
    }

    if (is_folder)
        drive_download_folder(T.access_token, id, name, local_dir);
    else
    {
        if (rfile_size > GDRIVE_UPLOAD_DOWNLOAD_FILE_SIZE_MAX)
        {
            SAP_ERR("GDrive Download: File (%s) is too big! (size: %llu) \n", name, rfile_size);
            ret = 1;
        }
        else
        {
            drive_download_file(T.access_token, id, name, local_dir);
            ret = 0;
        }
    }

    free(name);
    tokens_free(&T);
    return ret;
}

int gdrive_upload_one(const char *local_path, const char *parent_id)
{
    tokens_t T = {0};
    ensure_fresh_access_token(&T);
    if (!T.access_token || !*T.access_token)
    {
        tokens_free(&T);
        return 1;
    }

    char *newid = _drive_upload_file(T.access_token, local_path, NULL, parent_id, NULL);
    tokens_free(&T);
    if (!newid)
        return 1;
    free(newid);
    return 0;
}

int gdrive_id_to_name(const char *id, char *buf, size_t bufsz)
{
    if (!id || !buf || bufsz == 0)
        return 1;
    tokens_t T = {0};
    ensure_fresh_access_token(&T);
    if (!T.access_token || !*T.access_token)
    {
        tokens_free(&T);
        return 1;
    }

    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        tokens_free(&T);
        return 1;
    }

    snprintf(url, URL_MAX_SIZE, "%s/files/%s?fields=name", DRIVE_API, id);
    char *resp = curl_get_auth(url, T.access_token);
    url_free(url);
    tokens_free(&T);
    if (!resp)
    {
        SAP_ERR("<%s>: Invalid response!\n", __func__);
        return 1;
    }

    json_error_t je;
    json_t *J = json_loads(resp, 0, &je);
    free(resp);
    if (!J)
        return 1;
    const char *nm = json_string_value(json_object_get(J, "name"));
    snprintf(buf, bufsz, "%s", nm ? nm : "(unknown)");
    json_decref(J);
    return 0;
}

int gdrive_delete_one(const char *id)
{
    if (!id || !*id)
        return 1;

    tokens_t T = (tokens_t){0};
    ensure_fresh_access_token(&T);
    if (!T.access_token || !*T.access_token)
    {
        SAP_ERR("gdrive_delete_one: no access token (login/refresh failed)\n");
        tokens_free(&T);
        return 1;
    }

    CURL *C = curl_easy_init();
    if (!C)
    {
        SAP_ERR("gdrive_delete_one: curl init failed\n");
        tokens_free(&T);
        return 1;
    }
    set_common_curl_opts(C);

    /* Build URL: DELETE https://www.googleapis.com/drive/v3/files/{id}?supportsAllDrives=true */
    char *url = NULL;
    url = url_alloc();
    if (!url)
    {
        SAP_ERR("<%s>: No mem for URL!\n", __func__);
        curl_easy_cleanup(C);
        return 1;
    }

    snprintf(url, URL_MAX_SIZE, "%s/files/%s?supportsAllDrives=true", DRIVE_API, id);

    /* Headers */
    struct curl_slist *hdr = NULL;
    char auth[1024];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", T.access_token);
    hdr = curl_slist_append(hdr, auth);
    hdr = curl_slist_append(hdr, "Connection: close");

    /* Some servers may still send a small body; sink it to our write_cb */
    struct mem sink = {0};

    curl_easy_setopt(C, CURLOPT_URL, url);
    curl_easy_setopt(C, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(C, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(C, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(C, CURLOPT_WRITEDATA, &sink);

    CURLcode rc = curl_easy_perform(C);
    long code = 0;
    if (rc == CURLE_OK)
        curl_easy_getinfo(C, CURLINFO_RESPONSE_CODE, &code);

    url_free(url);

    if (sink.ptr)
        free(sink.ptr);
    curl_slist_free_all(hdr);
    curl_easy_cleanup(C);
    tokens_free(&T);

    /* Drive returns 204 No Content on success (accept 200 as well, just in case) */
    if (rc != CURLE_OK)
    {
        SAP_ERR("gdrive_delete_one: transport error: %s\n", curl_easy_strerror(rc));
        return 1;
    }
    if (code == 204 || code == 200)
    {
        SAP_INFO("Deleted: %s\n", id);
        return 0;
    }

    SAP_ERR("gdrive_delete_one: HTTP %ld deleting %s\n", code, id);
    return 1;
}

/********* Debug Functions *********/
void gdrive_test(SAP_CONFIG *sap_cfg)
{
    gdrive_init(sap_cfg);

    if (gdrive_login(NULL, NULL) == SUCCESS)
    {
        char *r_file_id = _gdrive_upload_file("/etc/hostname", "hostname.txt", "root", "text/plain");
        if (r_file_id)
        {
            SAP_INFO("Uploaded file (ID: %s)\n", r_file_id);
            if (_gdrive_list_rootdir() != SUCCESS)
            {
                SAP_ERR("list failed\n");
            }

            free(r_file_id);
        }
    }
    else
    {
        SAP_ERR("login failed\n");
    }

    gdrive_fini();
}
