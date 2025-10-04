/* memory_sync.h */
#ifndef _APP_UTILS_MEMORY_SYNC_H
#define _APP_UTILS_MEMORY_SYNC_H

#include <stdint.h>

#ifndef __ASSEMBLY__

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_ATOMICS__)
  /* C11 atomics available */
  #include <stdatomic.h>
  #define HAS_C11_ATOMICS 1
#else
  /* fallback to GCC/Clang built-in atomics */
  #define HAS_GNU_ATOMICS 1
#endif

/* --- 64-bit --- */
static inline uint64_t sync_read_u64(uint64_t *ptr) {
#if HAS_C11_ATOMICS
    return atomic_load_explicit((atomic_uint_least64_t*)ptr, memory_order_acquire);
#elif HAS_GNU_ATOMICS
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
#endif
}

static inline void sync_write_u64(uint64_t *ptr, uint64_t val) {
#if HAS_C11_ATOMICS
    atomic_store_explicit((atomic_uint_least64_t*)ptr, val, memory_order_release);
#elif HAS_GNU_ATOMICS
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
#endif
}

/* --- 32-bit --- */
static inline uint32_t sync_read_u32(uint32_t *ptr) {
#if HAS_C11_ATOMICS
    return atomic_load_explicit((atomic_uint_least32_t*)ptr, memory_order_acquire);
#elif HAS_GNU_ATOMICS
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
#endif
}

static inline void sync_write_u32(uint32_t *ptr, uint32_t val) {
#if HAS_C11_ATOMICS
    atomic_store_explicit((atomic_uint_least32_t*)ptr, val, memory_order_release);
#elif HAS_GNU_ATOMICS
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
#endif
}

/* --- 16-bit --- */
static inline uint16_t sync_read_u16(uint16_t *ptr) {
#if HAS_C11_ATOMICS
    return atomic_load_explicit((atomic_uint_least16_t*)ptr, memory_order_acquire);
#elif HAS_GNU_ATOMICS
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
#endif
}

static inline void sync_write_u16(uint16_t *ptr, uint16_t val) {
#if HAS_C11_ATOMICS
    atomic_store_explicit((atomic_uint_least16_t*)ptr, val, memory_order_release);
#elif HAS_GNU_ATOMICS
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
#endif
}

/* --- 8-bit --- */
static inline uint8_t sync_read_u8(uint8_t *ptr) {
#if HAS_C11_ATOMICS
    return atomic_load_explicit((atomic_uint_least8_t*)ptr, memory_order_acquire);
#elif HAS_GNU_ATOMICS
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
#endif
}

static inline void sync_write_u8(uint8_t *ptr, uint8_t val) {
#if HAS_C11_ATOMICS
    atomic_store_explicit((atomic_uint_least8_t*)ptr, val, memory_order_release);
#elif HAS_GNU_ATOMICS
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
#endif
}

static inline bool sync_read_bool(bool *ptr) {
    return (bool)sync_read_u8((uint8_t*)ptr);
}

static inline void sync_write_bool(bool *ptr, bool val) {
    sync_write_u8((uint8_t*)ptr, (uint8_t)val);
}

#endif // __ASSEMBLY__
#endif /* _APP_UTILS_MEMORY_SYNC_H */
