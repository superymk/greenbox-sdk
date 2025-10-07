// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOSM_USB_COMMON_UTILS_H
#define IOSM_USB_COMMON_UTILS_H

#ifndef __ASSEMBLY__

#define WORD_TO_UINT32(x)       ((uint32_t)(x))



extern bool usbpdev_addr_equal(USBPDEV_ADDR* addr1, USBPDEV_ADDR* addr2);
extern void usbpdev_addr_copy(USBPDEV_ADDR* to, USBPDEV_ADDR* from);

#endif // __ASSEMBLY__
#endif // IOSM_USB_COMMON_UTILS_H