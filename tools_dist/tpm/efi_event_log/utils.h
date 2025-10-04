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

#pragma once

#include <errno.h>
#include <stdbool.h>

#include "tcg2_protocol.h"

#ifndef __ASSEMBLY__

#define TRUE_STR "true"
#define FALSE_STR "false"

typedef bool (*DIGEST2_CALLBACK)(TCG_DIGEST2 *digest,
                                 void *data);

typedef bool (*EVENT2_CALLBACK)(TCG_EVENT_HEADER2 *event_hdr,
                                uint32_t event_idx,
                                void *data);

/// @brief Print a hex string.
/// @param buffer 
/// @param size 
extern void PrintHex(const uint8_t* buffer, size_t size);

/// @brief Gets the size of the algorithm based on its ID.
/// @param alg_id The algorithm ID.
/// @return The size of the algorithm in bytes.
extern size_t get_alg_size(uint16_t alg_id);

/// @brief Retrieves the next TCG_DIGEST2 structure.
/// @param digest The current digest.
/// @return A pointer to the next digest.
extern TCG_DIGEST2 *get_next_digest(TCG_DIGEST2 *digest);

/// @brief Calculates the size of a TCG_DIGEST2 structure.
/// @param digest The digest structure.
/// @return The size in bytes.
extern size_t sizeof_digest2(TCG_DIGEST2 *digest);

/// @brief Iterates over each digest in a TPM2 event header and applies a callback.
/// @param event_hdr The event header containing digests.
/// @param callback The callback function to apply.
/// @param data Additional data to pass to the callback.
/// @return true if iteration completed successfully, false otherwise.
extern bool foreach_digest2(TCG_EVENT_HEADER2 *event_hdr, DIGEST2_CALLBACK callback, void *data);

/// @brief Retrieves the next TPM2 event header from the current event.
/// @param event_hdr The current event header.
/// @return A pointer to the next event header, or NULL if none exists.
extern TCG_EVENT_HEADER2 *get_next_event(TCG_EVENT_HEADER2 *event_hdr);

/// @brief Iterates over a range of TPM2 event headers and applies a callback function.
/// @param event_first The first event header in the range.
/// @param event_last The last event header in the range.
/// @param callback The callback function to apply to each event.
/// @param data Additional data to pass to the callback.
/// @return true if all callbacks returned true, false otherwise.
extern bool foreach_event2(TCG_EVENT_HEADER2 *event_first, TCG_EVENT_HEADER2 *event_last, EVENT2_CALLBACK callback, void *data);

#endif // __ASSEMBLY__
