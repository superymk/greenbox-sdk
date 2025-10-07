// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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