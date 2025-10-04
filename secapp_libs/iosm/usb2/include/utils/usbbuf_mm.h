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

#ifndef _LIBUSB2_USBBUF_MM_H
#define _LIBUSB2_USBBUF_MM_H

#ifndef __ASSEMBLY__
struct sap_user_params;

typedef enum {
    USBBUF_MEM_DMA = 0x1,	// From DMA Pool
} USBBUF_MEM_TYPE;


extern SAP_STATUS usbbuf_create_pool(vaddr_t in_usbbuf_vaddr, size_t usbbuf_size);
extern void usbbuf_destroy_pool(void);

extern void* usbbuf_alloc_align(USBBUF_MEM_TYPE mem_type, uint32_t alignment, size_t size);
extern void* usbbuf_alloc(USBBUF_MEM_TYPE mem_type, size_t size);
extern void usbbuf_dealloc(USBBUF_MEM_TYPE mem_type, void* vaddr);

extern paddr_t usbbuf_get_base_paddr(void);

#endif // __ASSEMBLY__
#endif // _LIBUSB2_USBBUF_MM_H