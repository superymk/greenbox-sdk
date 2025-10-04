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

// must be the same with <sys/ipc/varbuf.h> in newlib

#ifndef VARIABLE_BUFFER_SERIALIZATION_DESERIALIZATION
#define VARIABLE_BUFFER_SERIALIZATION_DESERIALIZATION

// #include <inttypes.h>
#include <os/os_headers.h>
#include <app_types.h>
#include <stdbool.h>

#ifndef __ASSEMBLY__

typedef struct 
{
	uint32_t p_read;
	uint32_t p_write;

	uint8_t* data;
	void* private;
} varbuf_t;

/// @brief Start serialization or de-serialization on the given buffer <data>.
/// @param data 
/// @return 
extern varbuf_t* varbuf_access_start(uint8_t* data);

/// @brief Finish serialization or de-serialization.
/// @param varbuf 
extern void varbuf_access_finish(varbuf_t* varbuf);

/// @brief Start serialization or de-serialization on the given buffer <data>.
/// @param data 
/// @return 
extern varbuf_t* varbuf_access_start(uint8_t* data);

/// @brief Finish serialization or de-serialization.
/// @param varbuf 
extern void varbuf_access_finish(varbuf_t* varbuf);

/// @brief Serialize <var>. 
/// If <size> is <64 bytes, then the data is embedded in the SecProcess-Application communication message. Otherwise, the
/// data is placed on the SecProcess-Application shared memory (not in the communication channel).
/// @param varbuf 
/// @param var 
/// @param size 
/// @return 
extern bool varbuf_write_data(varbuf_t* varbuf, const void* var, const uint32_t size);

/// @brief Force serialize <var> in shared memory, regardless its size.
/// @param varbuf 
/// @param var 
/// @param size 
/// @return 
bool varbuf_write_data_smem(varbuf_t* varbuf, const void* var, const uint32_t size);

/// @brief Serialize string <str>.
/// If <size> is <64 bytes, then the data is embedded in the SecProcess-Application communication message. Otherwise, the
/// data is placed on the SecProcess-Application shared memory (not in the communication channel).
/// @param varbuf 
/// @param str 
/// @param max_len The value must be longer than strlen(str). Otherwise the terminator '\0' will not be sent.
/// @return 
extern bool varbuf_write_str(varbuf_t* varbuf, const char* str, size_t max_len);

/// @brief Force serialize string <str> in shared memory.
/// @param varbuf 
/// @param str 
/// @param max_len The value must be longer than strlen(str). Otherwise the terminator '\0' will not be sent.
/// @return 
extern bool varbuf_write_str_smem(varbuf_t* varbuf, const char* str, size_t max_len);

/// @brief Serialize NULL
/// @param varbuf 
/// @return 
extern bool varbuf_write_nil(varbuf_t* varbuf);

/// @brief Serialize an array
/// @param varbuf 
/// @param elems_count 
/// @return 
extern bool varbuf_write_array(varbuf_t* varbuf, const uint32_t elems_count);

/// @brief Return the size of the result serialization buffer, including the metadata.
/// @param varbuf 
/// @return 
extern uint32_t varbuf_size(varbuf_t* varbuf);

/// @brief Read the data size from <varbuf> without moving the read pointer in it.
/// The data could be placed in the SecProcess-Application communication message or SecProcess-Application shared memory, 
/// depending on its size.
/// @param varbuf 
/// @param datasz 
/// @return 
extern bool varbuf_tryread_datasz(varbuf_t* varbuf, uint32_t* datasz);

/// @brief Read the NIL value from <varbuf> without moving the read pointer in it.
/// @param varbuf 
/// @return 
extern bool varbuf_tryread_nil(varbuf_t* varbuf);

/// @brief De-serialize the data with unknown (variable) data length.
/// The data could be placed in the SecProcess-Application communication message or SecProcess-Application shared memory, 
/// depending on its size.
/// This function allocates the heap memory to store the data
/// @param varbuf 
/// @param datasz 
/// @return 
extern void* varbuf_read_vardata(varbuf_t* varbuf, uint32_t* datasz);

/// @brief De-serialize the data with unknown (variable) data length. 
/// The data could be placed in the SecProcess-Application communication message or SecProcess-Application shared memory, 
/// depending on its size.
/// No inner heap allocation.
/// @param varbuf 
/// @param data 
/// @param datasz 
/// @return 
extern bool varbuf_read_vardata_direct(varbuf_t* varbuf, void* data, uint32_t* datasz);

/// @brief De-serialize the data with known (fixed) data length. 
/// The data could be placed in the SecProcess-Application communication message or SecProcess-Application shared memory, 
/// depending on its size.
/// No inner heap allocation
/// @param varbuf 
/// @param data 
/// @param datasz 
/// @return 
extern bool varbuf_read_fixdata(varbuf_t* varbuf, void* data, uint32_t datasz);

/// @brief De-serialize the NULL.
/// @param varbuf 
/// @return 
extern bool varbuf_read_nil(varbuf_t* varbuf);

/// @brief De-serialize the array.
/// @param varbuf 
/// @param elems_count 
/// @return 
extern bool varbuf_read_array(varbuf_t* varbuf, uint32_t* elems_count);

/// @brief Write the status and the errno returned by the libc functions of the untrusted OS.
/// @param varbuf 
/// @param result 
/// @param err 
/// @return 
extern bool varbuf_util_write_result(varbuf_t* varbuf, uint64_t* result, int* err);

#endif // __ASSEMBLY__

#endif // VARIABLE_BUFFER_SERIALIZATION_DESERIALIZATION