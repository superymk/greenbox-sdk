// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

// Must be the same with between secos\ipc\varbuf.c in newlib and sp_loader\ipc\varbuf.c for loaders.

#include <string.h>
#include <stdlib.h>
#include <sys/ipc/varbuf.h>
#include <sys/ipc/comm.h>
#include "cmp.h"
#include "comm_ops.h"

#define DATA_EMBEDDED 1
#define DATA_IN_SHAREDMEM 2

#define MAX(a, b) \
    ({ __typeof__ (a) _a = (a); \
	  __typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; })

bool varbuf_reader(cmp_ctx_t *ctx, void *data, size_t size)
{
    varbuf_t *varbuf = (varbuf_t *)ctx->buf;
    uint32_t offset = varbuf->p_read;
    uint32_t new_offset = varbuf->p_read + size;

    if (!data)
        return false;

    // Avoid overflow of <new_offset>
    if ((new_offset < offset) || (new_offset < size))
        return false;

    memcpy(data, &varbuf->data[offset], size);
    varbuf->p_read += size;

    return true;
}

size_t varbuf_writer(cmp_ctx_t *ctx, const void *data, size_t size)
{
    varbuf_t *varbuf = (varbuf_t *)ctx->buf;
    uint32_t offset = varbuf->p_write;
    uint32_t new_offset = varbuf->p_write + size;

    if (!data)
        return 0;

    // Avoid overflow of <new_offset>
    if ((new_offset < offset) || (new_offset < size))
        return 0;

    memcpy(&varbuf->data[offset], data, size);
    varbuf->p_write += size;

    return size;
}

varbuf_t *varbuf_access_start(uint8_t *data)
{
    varbuf_t *varbuf;
    cmp_ctx_t *cmp;

    if (!data)
        return NULL;

    cmp = (cmp_ctx_t *)malloc(sizeof(cmp_ctx_t));
    varbuf = (varbuf_t *)malloc(sizeof(varbuf_t));

    varbuf->data = data;
    varbuf->private = cmp;
    varbuf->p_read = 0;
    varbuf->p_write = 0;

    cmp_init(cmp, varbuf, varbuf_reader, varbuf_writer);

    return varbuf;
}

void varbuf_access_finish(varbuf_t *varbuf)
{
    if (!varbuf)
        return;

    if (varbuf->private)
    {
        free(varbuf->private);
        varbuf->private = NULL;
    }

    free(varbuf);
}

//! Serialize <var>
bool varbuf_write_data(varbuf_t *varbuf, const void *var, const uint32_t size)
{
    cmp_ctx_t *cmp = NULL;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    if (size >= 64)
    {
        smem_offset_t offset = INVALID_SMEM_OFFSET;

        if (!cmp_write_u8(cmp, DATA_IN_SHAREDMEM)) // data store type (embedded/in shared mem)
            return false;
        if (!cmp_write_u32(cmp, size)) // size of data
            return false;

        offset = sp_comm_app_alloc_sharedmem(size);
        if (offset == INVALID_SMEM_OFFSET)
            return false;
        memcpy((void *)SMEM_OFFSET_TO_VADDR(offset), var, size);
        if (!cmp_write_u32(cmp, (uint32_t)offset)) // offset in shared memory
            return false;
    }
    else
    {
        if (!cmp_write_u8(cmp, DATA_EMBEDDED))
            return false;
        if (!cmp_write_bin(cmp, var, size))
            return false;
    }

    return true;
}

bool varbuf_write_data_smem(varbuf_t *varbuf, const void *var, const uint32_t size)
{
    cmp_ctx_t *cmp = NULL;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    {
        smem_offset_t offset = INVALID_SMEM_OFFSET;

        if (!cmp_write_u8(cmp, DATA_IN_SHAREDMEM)) // data store type (embedded/in shared mem)
            return false;
        if (!cmp_write_u32(cmp, size)) // size of data
            return false;

        offset = sp_comm_app_alloc_sharedmem(size);
        if (offset == INVALID_SMEM_OFFSET)
            return false;
        memcpy((void *)SMEM_OFFSET_TO_VADDR(offset), var, size);
        if (!cmp_write_u32(cmp, (uint32_t)offset)) // offset in shared memory
            return false;
    }

    return true;
}

//! Serialize string <str>
bool varbuf_write_str(varbuf_t *varbuf, const char *str, size_t max_len)
{
    size_t str_len = strnlen(str, max_len);

    if (str_len < max_len)
        return varbuf_write_data(varbuf, str, str_len + 1);
    else
        return varbuf_write_data(varbuf, str, max_len);
}

bool varbuf_write_str_smem(varbuf_t* varbuf, const char* str, size_t max_len)
{
	size_t str_len = strnlen(str, max_len);

	if(str_len < max_len)
		return varbuf_write_data_smem(varbuf, str, str_len + 1);
	else
		return varbuf_write_data_smem(varbuf, str, max_len);
}

//! Serialize NULL
bool varbuf_write_nil(varbuf_t *varbuf)
{
    cmp_ctx_t *cmp = NULL;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;
    if (!cmp_write_u8(cmp, DATA_EMBEDDED))
        return false;
    if (!cmp_write_nil(cmp))
        return false;
    return true;
}

//! Serialize an array
bool varbuf_write_array(varbuf_t *varbuf, const uint32_t elems_count)
{
    cmp_ctx_t *cmp = NULL;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;
    if (!cmp_write_u8(cmp, DATA_EMBEDDED))
        return false;
    if (!cmp_write_array(cmp, elems_count))
        return false;

    return true;
}

//! Return the size of the result serialization buffer, including the metadata.
uint32_t varbuf_size(varbuf_t *varbuf)
{
    return MAX(varbuf->p_read, varbuf->p_write);
}

//! Read the data size from <varbuf> without moving the read pointer in it.
bool varbuf_tryread_datasz(varbuf_t *varbuf, uint32_t *datasz)
{
    uint32_t orig_p_read = varbuf->p_read;
    cmp_ctx_t *cmp = NULL;
    bool result = false;
    uint8_t data_store_type = 0;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        goto out;

    // Get the data size
    if (data_store_type == DATA_EMBEDDED)
    {
        result = cmp_read_bin_size(cmp, datasz);
    }
    else if (data_store_type == DATA_IN_SHAREDMEM)
    {
        if (!cmp_read_u32(cmp, datasz))
            goto out;
        result = true;
    }

out:
    varbuf->p_read = orig_p_read;
    return result;
}

//! Read the NIL value from <varbuf> without moving the read pointer in it.
bool varbuf_tryread_nil(varbuf_t *varbuf)
{
    uint32_t orig_p_read = varbuf->p_read;
    cmp_ctx_t *cmp = NULL;
    bool result = false;
    uint8_t data_store_type = 0;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        goto out;

    if (!cmp_read_nil(cmp))
        goto out;

    result = true;

out:
    varbuf->p_read = orig_p_read;
    return result;
}

//! De-serialize the data with unknown (variable) data length
//! This function allocates the heap memory to store the data
void *varbuf_read_vardata(varbuf_t *varbuf, uint32_t *datasz)
{
    uint32_t size = 0;
    void *result = NULL;
    cmp_ctx_t *cmp = NULL;
    uint8_t data_store_type = 0;
    bool success = false;

    if (!varbuf)
        return NULL;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return NULL;

    // Get the data size
    if (!varbuf_tryread_datasz(varbuf, &size))
        goto out;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        goto out;

    result = malloc(size);

    // Get the data
    if (data_store_type == DATA_EMBEDDED)
    {
        if (!cmp_read_bin(cmp, result, &size))
            goto out;
    }
    else if (data_store_type == DATA_IN_SHAREDMEM)
    {
        smem_offset_t offset = INVALID_SMEM_OFFSET;
        uint32_t size2 = 0;

        if (!cmp_read_u32(cmp, &size2))
            goto out;

        if (size2 != size) // should always equal
            goto out;

        if (!cmp_read_u32(cmp, (uint32_t *)&offset)) // offset in shared memory
            goto out;
        memcpy(result, (void *)SMEM_OFFSET_TO_VADDR(offset), size);
        sp_comm_app_free_sharedmem(offset);
    }

    success = true;

out:
    if (datasz)
        *datasz = size;

    if (result && !success)
    {
        free(result);
        result = NULL;
    }

    return result;
}

//! De-serialize the data with unknown (variable) data length
//! No inner heap allocation
bool varbuf_read_vardata_direct(varbuf_t *varbuf, void *data, uint32_t *datasz)
{
    cmp_ctx_t *cmp = NULL;
    uint8_t data_store_type = 0;
    bool result = false;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        goto out;

    // Get the data
    if (data_store_type == DATA_EMBEDDED)
    {
        bool status = false;

        status = cmp_read_bin(cmp, data, datasz);
        if (!status)
            return status;
    }
    else if (data_store_type == DATA_IN_SHAREDMEM)
    {
        smem_offset_t offset = INVALID_SMEM_OFFSET;
        uint32_t size = 0;

        if (!cmp_read_u32(cmp, &size))
            goto out;

        if (!cmp_read_u32(cmp, (uint32_t *)&offset)) // offset in shared memory
            goto out;
        memcpy(data, (void *)SMEM_OFFSET_TO_VADDR(offset), size);
        sp_comm_app_free_sharedmem(offset);

        if (datasz)
            *datasz = size;
    }
    result = true;

out:
    return result;
}

bool varbuf_read_fixdata(varbuf_t *varbuf, void *data, uint32_t datasz)
{
    cmp_ctx_t *cmp = NULL;
    uint8_t data_store_type = 0;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        return false;

    // Get the data
    if (data_store_type == DATA_EMBEDDED)
    {
        if (!cmp_read_bin(cmp, data, &datasz))
            return false;
    }
    else if (data_store_type == DATA_IN_SHAREDMEM)
    {
        smem_offset_t offset = INVALID_SMEM_OFFSET;
        uint32_t size = 0;

        if (!cmp_read_u32(cmp, &size))
            return false;

        if (datasz > size)
            return false;

        if (!cmp_read_u32(cmp, (uint32_t *)&offset)) // offset in shared memory
            return false;

        memcpy(data, (void *)SMEM_OFFSET_TO_VADDR(offset), datasz);
        sp_comm_app_free_sharedmem(offset);
    }

    return true;
}

//! De-serialize the NULL
bool varbuf_read_nil(varbuf_t *varbuf)
{
    cmp_ctx_t *cmp = NULL;
    uint8_t data_store_type = 0;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp)
        return false;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        return false;

    if (!cmp_read_nil(cmp))
        return false;

    return true;
}

//! De-serialize the array
bool varbuf_read_array(varbuf_t *varbuf, uint32_t *elems_count)
{
    cmp_ctx_t *cmp = NULL;
    uint8_t data_store_type = 0;

    if (!varbuf)
        return false;

    cmp = (cmp_ctx_t *)varbuf->private;
    if (!cmp || !elems_count)
        return false;

    // Get the data store type
    if (!cmp_read_u8(cmp, &data_store_type))
        return false;

    if (!cmp_read_array(cmp, elems_count))
        return false;

    return true;
}

//! Read the status and the errno returned by the libc functions of the untrusted OS
bool varbuf_util_read_result(varbuf_t *varbuf, uint64_t *result, int *err)
{
    bool status = false;

    // Read result
    if (result)
    {
        status = varbuf_read_fixdata(varbuf, result, sizeof(uint64_t));
        if (!status)
            return false;
    }

    // Read errno
    if (err)
    {
        status = varbuf_read_fixdata(varbuf, err, sizeof(int));
        if (!status)
            return false;
    }

    return true;
}

/********* Public Functions - Utils for Types *********/
bool varbuf_serialize_size_t(varbuf_t *varbuf, size_t size)
{
    uint64_t size_temp = (uint64_t)size; 

    if (!varbuf_write_data(varbuf, &size_temp, sizeof(uint64_t))) // serialize size_t as uint64_t
    {
       return false;
    }

    // On success
    return true;
}

bool varbuf_deserialize_size_t(varbuf_t *varbuf, size_t* out_size)
{
    uint64_t size_temp = 0;

    if (!varbuf_read_fixdata(varbuf, &size_temp, sizeof(uint64_t)))
    {
        return false;
    }

    if (out_size)
    	*out_size = (size_t)size_temp;

    // On success
    return true;
}