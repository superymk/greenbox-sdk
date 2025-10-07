/*	$OpenBSD: bus.h,v 1.35 2020/10/28 09:58:57 jsg Exp $	*/
/*	$NetBSD: bus.h,v 1.6 1996/11/10 03:19:25 thorpej Exp $	*/

/*-
 * Copyright (c) 1996, 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1996 Charles M. Hannum.  All rights reserved.
 * Copyright (c) 1996 Jason R. Thorpe.  All rights reserved.
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Christopher G. Demetriou
 *	for the NetBSD Project.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE_BUS_H_
#define _MACHINE_BUS_H_

#include <sys/types.h>

/*
 * Bus access types.
 */
typedef u_long bus_addr_t;
typedef u_long bus_size_t;
typedef u_long bus_space_handle_t;

// [GreenBox-Specific] Dummy type, because it is unnecessary for GreenBox DMA APIs.
typedef unsigned long bus_space_tag_t;
typedef unsigned long bus_dma_tag_t;
// [GreenBox-Specific] End.

// [GreenBox-Specific] MMIO read reimplemented with GreenBox APIs
void gb_bus_fill_pci_struct(vaddr_t in_pci_mmio_base);
u_int8_t bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset);
u_int16_t bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset);
u_int32_t bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset);
void bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, u_int8_t value);
void bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, u_int16_t value);
void bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, u_int32_t value);
// [GreenBox-Specific] End.
/*
 * Flags used in various bus DMA methods.
 */
#define	BUS_DMA_WAITOK		0x0000	/* safe to sleep (pseudo-flag) */
#define	BUS_DMA_NOWAIT		0x0001	/* not safe to sleep */
#define	BUS_DMA_ALLOCNOW	0x0002	/* perform resource allocation now */
#define	BUS_DMA_COHERENT	0x0004	/* hint: map memory DMA coherent */
#define	BUS_DMA_BUS1		0x0010	/* placeholders for bus functions... */
#define	BUS_DMA_BUS2		0x0020
#define	BUS_DMA_32BIT		0x0040
#define	BUS_DMA_24BIT		0x0080	/* isadma map */
#define	BUS_DMA_STREAMING	0x0100	/* hint: sequential, unidirectional */
#define	BUS_DMA_READ		0x0200	/* mapping is device -> memory only */
#define	BUS_DMA_WRITE		0x0400	/* mapping is memory -> device only */
#define	BUS_DMA_NOCACHE		0x0800	/* map memory uncached */
#define	BUS_DMA_ZERO		0x1000	/* zero memory in dmamem_alloc */
#define	BUS_DMA_64BIT		0x2000	/* device handles 64bit dva */

/*
 * Operations performed by bus_dmamap_sync().
 */
#define BUS_DMASYNC_PREREAD	0x01
#define BUS_DMASYNC_POSTREAD	0x02
#define BUS_DMASYNC_PREWRITE	0x04
#define BUS_DMASYNC_POSTWRITE	0x08

/*
 *	bus_dma_segment_t
 *
 *	Describes a single contiguous DMA transaction.  Values
 *	are suitable for programming into DMA registers.
 */
struct bus_dma_segment {
	bus_addr_t	ds_addr;	/* DMA address */
	bus_size_t	ds_len;		/* length of transfer */
    /*
	 * Ugh. need this so can pass alignment down from bus_dmamem_alloc
	 * to scatter gather maps. only the first one is used so the rest is
	 * wasted space. bus_dma could do with fixing the api for this.
	 */
	bus_size_t	_ds_boundary;	/* don't cross */
	bus_size_t	_ds_align;	/* align to me */

	// [GreenBox-Specific] <smalloc_align> returns virtual addresses of DMA buffers. So <bus_dmamem_map> only needs to
    // read out the mapping.
	paddr_t		_ds_paddr;	/* CPU address */
	vaddr_t		_ds_vaddr;	/* CPU address */
	// [GreenBox-Specific] End
};
typedef struct bus_dma_segment	bus_dma_segment_t;

/*
 *	bus_dmamap_t
 *
 *	Describes a DMA mapping.
 */
struct bus_dmamap {
	/*
	 * PUBLIC MEMBERS: these are used by machine-independent code.
	 */
	bus_size_t	dm_mapsize;	/* size of the mapping */
	int		dm_nsegs;	/* # valid segments in mapping */
	bus_dma_segment_t dm_segs[1];	/* segments; variable length */
};
typedef struct bus_dmamap		*bus_dmamap_t;


/*
 * Common function for DMA map creation.  May be called by bus-specific
 * DMA map creation functions.
 */
int
bus_dmamap_create(bus_dma_tag_t t, bus_size_t size, int nsegments,
    bus_size_t maxsegsz, bus_size_t boundary, int flags, bus_dmamap_t *dmamp);

/*
 * Like _bus_dmamap_load(), but for raw memory allocated with
 * bus_dmamem_alloc().
 */
int
bus_dmamap_load_raw(bus_dma_tag_t t, bus_dmamap_t map, bus_dma_segment_t *segs,
    int nsegs, bus_size_t size, int flags);

/*
 * Common function for DMA map synchronization.  May be called
 * by bus-specific DMA map synchronization functions.
 */
void
bus_dmamap_sync(bus_dma_tag_t t, bus_dmamap_t map, bus_addr_t addr,
    bus_size_t size, int op);

/*
 * Common function for mapping DMA-safe memory.  May be called by
 * bus-specific DMA memory map functions.
 */
int
bus_dmamem_map(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs,
    size_t size, caddr_t *kvap, int flags);

/*
 * Common function for DMA-safe memory allocation.  May be called
 * by bus-specific DMA memory allocation functions.
 */
int
bus_dmamem_alloc(bus_dma_tag_t t, bus_size_t size, bus_size_t alignment,
    bus_size_t boundary, bus_dma_segment_t *segs, int nsegs, int *rsegs,
    int flags);

/*
 * Common function for unmapping DMA-safe memory.  May be called by
 * bus-specific DMA memory unmapping functions.
 */
void
bus_dmamem_unmap(bus_dma_tag_t t, caddr_t kva, size_t size);

/*
 * Common function for freeing DMA-safe memory.  May be called by
 * bus-specific DMA memory free functions.
 */
void
bus_dmamem_free(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs);

/*
 * Common function for DMA map destruction.  May be called by bus-specific
 * DMA map destruction functions.
 */
void
bus_dmamap_destroy(bus_dma_tag_t t, bus_dmamap_t map);

/*
 * Common function for unloading a DMA map.  May be called by
 * bus-specific DMA map unload functions.
 */
void
bus_dmamap_unload(bus_dma_tag_t t, bus_dmamap_t map);

#endif /* _MACHINE_BUS_H_ */
