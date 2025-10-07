// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file [NOTE] We do not name the file as "semaphore.c" because newlib-4.5.0 seems to have bugs to link all C files with the
/// same file name though they are in different sub-folders in newlib/libc/sys/secos. 

// If you already have these in headers, include them instead of the forward decls.
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/unistd.h>
#include <fcntl.h>

#ifdef __i386__
#include <sys/machine/i386/atomic.h>
#elif defined __amd64__
#include <sys/machine/amd64/atomic.h>
#endif

static inline void _sem_lock(sem_t *s)   { __lock_acquire(s->_Semaphore.lock); }
static inline void _sem_unlock(sem_t *s) { __lock_release(s->_Semaphore.lock); }

static inline bool _sem_valid(sem_t *s) {
    return s && s->_Semaphore.magic == SEM_MAGIC && (s->_flags & SEM_F_INITED);
}

static inline void _sleep_short_backoff(unsigned long *us, unsigned long limit_us) {
    // exponential-ish backoff capped at limit
    unsigned long next = (*us < 64 ? 64 : (*us > limit_us ? limit_us : (*us * 2)));
    *us = next > limit_us ? limit_us : next;
}

// Convert relative timespec to microseconds (cap at ~years to avoid overflow).
static unsigned long long _ts_rel_to_us(const struct timespec *rel) {
    if (!rel) return 0ULL;
    unsigned long long s = (unsigned long long)(rel->tv_sec >= 0 ? rel->tv_sec : 0);
    unsigned long long ns = (unsigned long long)(rel->tv_nsec >= 0 ? rel->tv_nsec : 0);
    if (ns >= 1000000000ULL) ns = 999999999ULL;
    // guard overflow: cap to ~1e18/us if needed, but our loops consume it anyway
    unsigned long long us = s * 1000000ULL + ns / 1000ULL;
    return us;
}

// -------- Named semaphore registry (in-process, simple) --------
// Minimal in-memory registry; adapt to your shared-object or kernel if needed.
typedef struct named_sem_entry {
    char                   *name;
    sem_t                   sem;      // stored by value; address stable
    volatile int            refcnt;
    volatile int            unlinked; // boolean
    struct named_sem_entry *next;
} named_sem_entry;

static _LOCK_T g_sem_table_lock;
static named_sem_entry *g_sem_table_head = NULL;
static int g_sem_table_inited = 0;

static void _sem_table_init_once(void) {
    if (!g_sem_table_inited) {
        __lock_init(g_sem_table_lock);
        g_sem_table_inited = 1;
    }
}

// Simple lookup; caller holds g_sem_table_lock
static named_sem_entry *_sem_table_find(const char *name) {
    for (named_sem_entry *e = g_sem_table_head; e; e = e->next) {
        if (strcmp(e->name, name) == 0) return e;
    }
    return NULL;
}

// ---------- Core operations ----------
static int _sem_do_wait(sem_t *s, unsigned long long rel_timeout_us, bool is_timed, bool try_only) {
    if (!_sem_valid(s)) { errno = EINVAL; return -1; }

    // fast path try: single locked check
    _sem_lock(s);
    if (s->_Semaphore.count > 0) {
        s->_Semaphore.count--;
        _sem_unlock(s);
        return 0;
    }
    if (try_only) {
        _sem_unlock(s);
        errno = EAGAIN;
        return -1;
    }
    // track waiter count (approximate; protected by lock)
    s->_Semaphore.waiters++;
    _sem_unlock(s);

    unsigned long long remaining = rel_timeout_us;
    unsigned long sleep_backoff_us = 64;   // start small, back off
    const unsigned long max_backoff_us = 2000; // 2ms upper bound for polling granularity

    for (;;) {
        // Try acquire
        _sem_lock(s);
        if (s->_Semaphore.count > 0) {
            s->_Semaphore.count--;
            s->_Semaphore.waiters--;
            _sem_unlock(s);
            return 0;
        }
        _sem_unlock(s);

        // Not available; decide to sleep
        if (is_timed) {
            if (remaining == 0) {
                // timeout
                _sem_lock(s);
                if (s->_Semaphore.waiters > 0) s->_Semaphore.waiters--;
                _sem_unlock(s);
                errno = ETIMEDOUT;
                return -1;
            }
        }

        // Sleep a bit to yield CPU; mix of short usleep and occasional 1s sleeps for very long waits.
        unsigned long chunk_us = sleep_backoff_us;
        if (is_timed) {
            if (remaining < (unsigned long long)chunk_us) chunk_us = (unsigned long)remaining;
        }

        if (chunk_us >= 1000000UL) {
            // very long timeout: sleep by seconds to avoid large usleep values
            unsigned int secs = chunk_us / 1000000UL;
            if (secs == 0) secs = 1;
            sleep(secs);
        } else {
            if (chunk_us == 0) chunk_us = 64;
            usleep(chunk_us);
        }

        if (is_timed) {
            if (chunk_us >= 1000000UL) {
                unsigned long long deducted = ((unsigned long long)(chunk_us / 1000000UL)) * 1000000ULL;
                if (remaining >= deducted) remaining -= deducted; else remaining = 0;
            } else {
                if (remaining >= chunk_us) remaining -= chunk_us; else remaining = 0;
            }
        }

        _sleep_short_backoff(&sleep_backoff_us, max_backoff_us);
    }
}

// ---------- Public API ----------

int sem_init(sem_t *s, int pshared, unsigned int value) {
    (void)pshared; // this in-process version ignores pshared (no cross-process share)
    if (!s) { errno = EINVAL; return -1; }
    if (value > UINT_MAX) { errno = EINVAL; return -1; }

    __lock_init(s->_Semaphore.lock);
    s->_Semaphore.max = UINT_MAX;
    s->_Semaphore.count = value;
    s->_Semaphore.waiters = 0;
    s->_Semaphore.magic = SEM_MAGIC;
    s->_flags = SEM_F_INITED; // unnamed by default
    return 0;
}

int sem_destroy(sem_t *s) {
    if (!_sem_valid(s)) { errno = EINVAL; return -1; }
    if (s->_flags & SEM_F_NAMED) { errno = EINVAL; return -1; } // use sem_close/sem_unlink

    _sem_lock(s);
    if (s->_Semaphore.waiters > 0) {
        _sem_unlock(s);
        errno = EBUSY;
        return -1;
    }
    s->_Semaphore.magic = 0;
    s->_flags = 0;
    _sem_unlock(s);
    return 0;
}

int sem_post(sem_t *s) {
    if (!_sem_valid(s)) { errno = EINVAL; return -1; }

    _sem_lock(s);
    if (s->_Semaphore.count == s->_Semaphore.max) {
        _sem_unlock(s);
        errno = EOVERFLOW;
        return -1;
    }
    s->_Semaphore.count++;
    _sem_unlock(s);
    // No explicit wake: waiters poll and will observe count>0
    return 0;
}

int sem_trywait(sem_t *s) {
    return _sem_do_wait(s, 0, false, true);
}

int sem_wait(sem_t *s) {
    return _sem_do_wait(s, 0, false, false);
}

int sem_timedwait(sem_t *s, const struct timespec *timeout) {
    // IMPORTANT: treated as RELATIVE timeout (duration) here.
    if (!timeout || (timeout->tv_sec < 0) || (timeout->tv_nsec < 0)) {
        errno = EINVAL; return -1;
    }
    unsigned long long us = _ts_rel_to_us(timeout);
    return _sem_do_wait(s, us, true, false);
}

int sem_getvalue(sem_t *s, int *sval) {
    if (!_sem_valid(s) || !sval) { errno = EINVAL; return -1; }
    _sem_lock(s);
#if SEM_GETVALUE_NEGATIVE_WAITERS
    if (s->_Semaphore.count == 0 && s->_Semaphore.waiters > 0) {
        *sval = -s->_Semaphore.waiters;
    } else {
        *sval = (int)s->_Semaphore.count;
    }
#else
    *sval = (int)s->_Semaphore.count;
#endif
    _sem_unlock(s);
    return 0;
}

// Named semaphores

// Return: pointer to the stored sem inside the registry
sem_t *sem_open(const char *name, int oflag, ...) {
    if (!name || !*name) { errno = EINVAL; return SEM_FAILED; }

    _sem_table_init_once();

    unsigned int initval = 0;
    bool have_init = false;

    if (oflag & O_CREAT) {
        // Parse mode, value
        va_list ap;
        va_start(ap, oflag);
        (void)va_arg(ap, unsigned int); // mode_t mode; ignored in this in-process impl
        initval = va_arg(ap, unsigned int);
        va_end(ap);
        have_init = true;
    }

    // Lock table
    __lock_acquire(g_sem_table_lock);

    named_sem_entry *e = _sem_table_find(name);
    if (e) {
        if ((oflag & O_CREAT) && (oflag & O_EXCL)) {
            __lock_release(g_sem_table_lock);
            errno = EEXIST;
            return SEM_FAILED;
        }
        atomic_increase(&e->refcnt);
        __lock_release(g_sem_table_lock);
        return &e->sem;
    }

    // Not found
    if (!(oflag & O_CREAT)) {
        __lock_release(g_sem_table_lock);
        errno = ENOENT;
        return SEM_FAILED;
    }

    // Create new entry
    e = (named_sem_entry *)calloc(1, sizeof(*e));
    if (!e) {
        __lock_release(g_sem_table_lock);
        errno = ENOMEM;
        return SEM_FAILED;
    }
    e->name = strdup(name);
    if (!e->name) {
        __lock_release(g_sem_table_lock);
        free(e);
        errno = ENOMEM;
        return SEM_FAILED;
    }

    // Initialize the underlying semaphore
    __lock_init(e->sem._Semaphore.lock);
    e->sem._Semaphore.max = UINT_MAX;
    e->sem._Semaphore.count = have_init ? initval : 0;
    e->sem._Semaphore.waiters = 0;
    e->sem._Semaphore.magic = SEM_MAGIC;
    e->sem._flags = SEM_F_INITED | SEM_F_NAMED;

    e->refcnt = 1;
    e->unlinked = 0;

    // Insert at head
    e->next = g_sem_table_head;
    g_sem_table_head = e;

    __lock_release(g_sem_table_lock);
    return &e->sem;
}

int sem_close(sem_t *s) {
    if (!_sem_valid(s)) { errno = EINVAL; return -1; }
    if (!(s->_flags & SEM_F_NAMED)) { errno = EINVAL; return -1; }

    _sem_table_init_once();
    __lock_acquire(g_sem_table_lock);

    // Find the entry containing this sem
    named_sem_entry **pp = &g_sem_table_head;
    named_sem_entry *e = NULL;
    for (; *pp; pp = &(*pp)->next) {
        if (&((*pp)->sem) == s) { e = *pp; break; }
    }
    if (!e) {
        __lock_release(g_sem_table_lock);
        errno = EINVAL;
        return -1;
    }

    atomic_decrease(&e->refcnt);
    // If refcnt hits 0 AND unlinked, remove and free.
    if (e->refcnt <= 0 && e->unlinked) {
        *pp = e->next;
        // poison
        e->sem._Semaphore.magic = 0;
        e->sem._flags = 0;
        free(e->name);
        free(e);
    }

    __lock_release(g_sem_table_lock);
    return 0;
}

int sem_unlink(const char *name) {
    if (!name || !*name) { errno = EINVAL; return -1; }

    _sem_table_init_once();
    __lock_acquire(g_sem_table_lock);

    named_sem_entry **pp = &g_sem_table_head;
    for (; *pp; pp = &(*pp)->next) {
        named_sem_entry *e = *pp;
        if (strcmp(e->name, name) == 0) {
            e->unlinked = 1;
            if (e->refcnt <= 0) {
                // remove immediately
                *pp = e->next;
                e->sem._Semaphore.magic = 0;
                e->sem._flags = 0;
                free(e->name);
                free(e);
            }
            __lock_release(g_sem_table_lock);
            return 0;
        }
    }
    __lock_release(g_sem_table_lock);
    errno = ENOENT;
    return -1;
}