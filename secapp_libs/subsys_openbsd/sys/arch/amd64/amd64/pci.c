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

#include <sddk.h>

// #define SKIP_OPENBSD_RENAME
#include <subsys_openbsd.h>

#include <sys/types.h>
#include <machine/bus.h>

static vaddr_t pci_mmio_base = INVALID_VADDR;

// [GreenBox-Specific] MMIO read reimplemented with GreenBox APIs

void gb_bus_fill_pci_struct(vaddr_t in_pci_mmio_base)
{
    pci_mmio_base = in_pci_mmio_base;
}

u_int8_t bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset)
{
    return read8(pci_mmio_base + offset);
}

u_int16_t bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset)
{
    return read16(pci_mmio_base + offset);
}

u_int32_t bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset)
{
    return read32(pci_mmio_base + offset);
}

void bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, u_int8_t value)
{
    write8(pci_mmio_base + offset, value);
}

void bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, u_int16_t value)
{
    write16(pci_mmio_base + offset, value);
}

void bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, u_int32_t value)
{
    write32(pci_mmio_base + offset, value);
}

// [GreenBox-Specific] End.