// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Zongwei Zhou (Carnegie Mellon University)
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef _PAL_UTIL
#define _PAL_UTIL

// #include "pal_string.h"

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
extern u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
extern u16 __segment_FS, __segment_GS;


/************ memory management ****************/
#define memset_far(d_seg, d_far, c, len) pal_mm_memset(d_far, c, len)
#define memset16_far(d_seg, d_far, c, len) pal_mm_memset(d_far, c, len)
#define memset_fl(ptr, val, size) pal_mm_memset(ptr, val, size)
#define memcpy_far(d_seg, d_far, s_seg, s_far, len) pal_mm_memcpy(d_far, s_far, len)
#define memcpy_fl(d_fl, s_fl, len) pal_mm_memcpy(d_fl, s_fl, len)


// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, value)                                   \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_ ## SEG)       \
            : "rm"(value))
#define __GET_SEG(SEG) ({                                       \
    u16 __seg;                                                  \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(__seg)               \
            : "m"(__segment_ ## SEG));                          \
    __seg;})

    
#define MAKE_FLATPTR(seg,off) ((void*)(((u32)(seg)<<4)+(u32)(off)))
// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, value) __SET_SEG(SEG, (value))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))
#endif //_PAL_UTIL
