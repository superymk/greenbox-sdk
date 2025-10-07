// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef LIBUSB2_EEHCI_MGR_H
#define LIBUSB2_EEHCI_MGR_H

#define EEHCI_SLOTS              (eEHCI_INSTANCE_NUM)




#ifndef __ASSEMBLY__

extern void eehci_mgr_init(void);
extern void eehci_mgr_fini(void);

extern SAP_STATUS eehci_register(uint32_t slot, union eEHCI_ID eehci_id, uint32_t eehci_handle);
extern SAP_STATUS eehci_unregister(uint32_t slot);

/// @brief Return the eEHCI info stored at slot <slot>
extern SAP_STATUS eehci_get_info(uint32_t slot, union eEHCI_ID* out_eehci_id, uint32_t* out_eehci_handle);

#endif // __ASSEMBLY__
#endif // LIBUSB2_EEHCI_MGR_H