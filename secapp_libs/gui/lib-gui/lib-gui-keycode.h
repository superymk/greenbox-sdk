// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIB_GUI_KEYCODE_H
#define LIB_GUI_KEYCODE_H

#ifndef __ASSEMBLY__

/// @brief Convert Libhid keycode to LVGL keycode
/// @param libhid_keycode 
/// @return 
extern uint32_t libhid_keycode_to_lvgl_keycode(uint32_t libhid_keycode);


#endif // __ASSEMBLY__
#endif // LIB_GUI_KEYCODE_H