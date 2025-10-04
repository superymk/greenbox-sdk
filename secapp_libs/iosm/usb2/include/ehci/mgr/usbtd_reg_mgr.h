/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#ifndef LIBUSB2_USBTD_REG_MGR_H
#define LIBUSB2_USBTD_REG_MGR_H

#define USBTD_REG_NUMS              (eEHCI_USBTD_SlotID_NUMS)

#define USBTD_REG_VALID_SLOT_MIN     0
#define USBTD_REG_VALID_SLOT_MAX     (USBTD_REG_NUMS - 1)


#define USBTD_REG_SLOT_INVALID           (-1)


#ifndef __ASSEMBLY__


#define IS_USBTD_REG_SLOT_VALID(reg_slot)   (VALUE_IN_RANGE_INCLUSIVE((reg_slot), USBTD_REG_VALID_SLOT_MIN, USBTD_REG_VALID_SLOT_MAX))

extern void usbtd_reg_mgr_init(void);
extern void usbtd_reg_mgr_fini(void);

extern usbtd_reg_slot_t usbtd_reg_pick(void);
extern void usbtd_reg_return(usbtd_reg_slot_t reg_slot);


#endif // __ASSEMBLY__
#endif // LIBUSB2_USBTD_REG_MGR_H