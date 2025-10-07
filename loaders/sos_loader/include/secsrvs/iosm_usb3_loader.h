// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef IOSM_USB3_LOADER_H
#define IOSM_USB3_LOADER_H

#include <appddk.h>

#ifndef __ASSEMBLY__

// /// @brief Re-registering IOSM-USB3 does not access EHCI's PCI config space prior to calling this function. This allows users to
// /// restart IOSM-USB3 without restarting IOSM-PCIE, which exclusively owns the I/O ports to access all PCI config spaces.
// /// After IOSM-PCIE quits (i.e., all I/O separation kernels quit), one should call this function this free infos stored
// /// in SecAgent. 
// /// @param  
// extern void iosm_usb3_unprobe_resc_info(void);

int _iosm_usb3_load(SL_CFG *sos_cfg);

int _iosm_usb3_run(void);

void iosm_usb3_terminate(void);

int _iosm_usb3_unload(void);

#endif // __ASSEMBLY__
#endif // IOSM_USB3_LOADER_H