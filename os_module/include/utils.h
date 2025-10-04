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

#ifndef SECAGENT_UTILS_H
#define SECAGENT_UTILS_H

#include <os_types.h>

#include <os_funcs.h>

#ifndef __ASSEMBLY__

//! This struct abstract the required common OS functions
extern struct os_funcs *os;

/// @brief [NOTE] <write32> is NOT <writel>. <write32> takes uint32_t, but <writel> takes ulong_t.
/// @param addr 
/// @param val 
static inline void sa_write32(vaddr_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;

    // Prevent reading stale values of other memory/register impacted from the given memory/register <addr>
    os->mm_fence();
}

static inline void sa_write16(vaddr_t addr, uint16_t val) {
    *(volatile uint16_t *)addr = val;

    // Prevent reading stale values of other memory/register impacted from the given memory/register <addr>
    os->mm_fence();
}

static inline void sa_write8(vaddr_t addr, uint8_t val) {
    *(volatile uint8_t *)addr = val;

    // Prevent reading stale values of other memory/register impacted from the given memory/register <addr>
    os->mm_fence();
}

/// @brief [NOTE] <read32> is NOT <readl>. <read32> returns uint32_t, but <readl> returns ulong_t.
/// @param addr 
/// @return 
static inline uint32_t sa_read32(const vaddr_t addr) 
{
    return *(volatile const uint32_t *)addr;
}
static inline uint16_t sa_read16(const vaddr_t addr) 
{
    return *(volatile const uint16_t *)addr;
}
static inline uint8_t sa_read8(const vaddr_t addr) 
{
    return *(volatile const uint8_t *)addr;
}

/// @brief Set the bits set in <mask>.
static inline uint32_t uint32_clear_bits(uint32_t value, uint32_t mask) {
	return (value & ~mask);
}

/// @brief Clear the bits set in <mask>.
static inline uint32_t uint32_set_bits(uint32_t value, uint32_t mask) {
	return (value | mask);
}

/// @brief Return the bits masked by <mask>
static inline uint32_t uint32_get_bits(uint32_t value, uint32_t mask) {
	return (value & mask);
}

/// @brief Set the bit <n> of <val>.
static inline uint32_t uint32_bit_set(uint32_t val, uint32_t n)
{
    return (val | (1 << n));
}

/// @brief Clear the bit <n> of <val>.
static inline uint32_t uint32_bit_clear(uint32_t val, uint32_t n)
{
    return (val & (~(1 << n)));
}

/// @brief Return TRUE if the bit <n> in <val> is 1.
static inline bool uint32_bit_test_set(uint32_t val, uint32_t n)
{
    return (val & (1 << n)) ? true : false;
}

/// @brief Return TRUE if the bit <n> in <val> is 0.
static inline bool uint32_bit_test_clear(uint32_t val, uint32_t n)
{
    return !uint32_bit_test_set(val, n);
}

// @brief Put the highest bit of <v> to <ans>.
// @return true if successful, false if <v> is 0.
extern bool ulong_test_highest_bit(ulong_t v, uint32_t *ans);

#endif // __ASSEMBLY__
#endif // SECAGENT_UTILS_H
