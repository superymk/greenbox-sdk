// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#pragma once

// GUID for SecAgent WPP tracing: {4d6e4677-9b06-4260-b32d-3f2dbd24ebac}
// WPP_DEFINE_BIT allows setting debug bit masks to selectively print.   
// The names defined in the WPP_DEFINE_BIT call define the actual names 
// that are used to control the level of tracing for the control guid 
// specified. 
#define WPP_CHECK_FOR_NULL_STRING  //to prevent exceptions due to NULL strings

#define WPP_CONTROL_GUIDS                                            \
    WPP_DEFINE_CONTROL_GUID( FileIoTraceGuid,                        \
                             (4d6e4677,9b06,4260,b32d,3f2dbd24ebac), \
                             WPP_DEFINE_BIT(DBG_INIT)     \
                             WPP_DEFINE_BIT(DBG_RW)       \
                             WPP_DEFINE_BIT(DBG_IOCTL)    \
                             )                                    

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level  >= lvl)

#pragma warning(disable:4204) // C4204 nonstandard extension used : non-constant aggregate initializer