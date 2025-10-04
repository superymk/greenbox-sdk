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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <wolftpm/tpm2.h>
#include <wolftpm/tpm2_wrap.h>

#include "./cJSON/cJSON.h"
#include "base64.h"

#include <math.h>
#include <log.h>

#include "cmds.h"
#include "app_os_if.h"
#include "utils_string.h"
#include "utils.h"
#include "hypercall.h"
#include "./hash/hash.h"

/// @brief Open and parse the JSON file <filepath>.
/// @param filepath Path to the JSON file.
/// @param create_if_not_exist Flag indicating whether to create the file if it does not exist.
/// @param out_file_json Pointer to store the resulting cJSON object.
/// @return 0 on success, non-zero on failure.
static int _json_file_read_all(const char *filepath, bool create_if_not_exist, cJSON **out_file_json)
{
    int ret = 0;
    uint8_t *file_buf = NULL;
    size_t file_len = 0;
    cJSON *file_json = NULL;

    // Attempt to read the file
    ret = fs_read_file(filepath, create_if_not_exist, &file_buf, &file_len);
    if (ret)
    {
        return 1; // Failed to read the file
    }

    // Attempt to parse the JSON content
    if (file_len > 0)
    {
        file_json = cJSON_Parse((char *)file_buf);
        if (!file_json)
        {
            // Parsing failed, return error
            if (!create_if_not_exist)
            {
                free(file_buf);
                return 1;
            }
        }
    }
    else
    {
        // File is empty
        if (!create_if_not_exist)
        {
            free(file_buf);
            return 1;
        }
    }

    // If creation is allowed, create an empty JSON object
    if (!file_json && create_if_not_exist)
    {
        file_json = cJSON_CreateObject();
        if (!file_json)
        {
            free(file_buf);
            return 1; // Failed to create a new JSON object
        }
    }

    // Clean up the file buffer as it's no longer needed
    if (file_buf)
    {
        free(file_buf);
    }

    // Assign the resulting JSON object to the output parameter
    *out_file_json = file_json;
    return 0; // Success
}

/// @brief Write JSON data to the file <filepath> and close it.
/// @param filepath 
/// @param file_json 
/// @return 
static int _json_file_overwrite(const char *filepath, cJSON* file_json)
{
    int ret = 0;
    char* file_buf = NULL;
    
    file_buf = cJSON_Print(file_json);
    if(!file_buf)
    {
        ret = 1;
        goto err;
    }

    ret = fs_write_file(filepath, (uint8_t*)file_buf, strlen(file_buf));
    if(ret)
    {
        ret = 1;
        goto err;
    }
    
    cJSON_free(file_buf);
    cJSON_Delete(file_json);
    return 0;

err:
    if(file_buf)
        cJSON_free(file_buf);
    cJSON_Delete(file_json);
    return ret;
}

/// @brief Create an empty "sealed_objects" array.
/// @param file_json 
/// @return 
static int _sealed_objs_file_create_empty_json_root(cJSON* file_json, cJSON** out_all_objs_json)
{
    cJSON *all_objs_json = cJSON_GetObjectItemCaseSensitive(file_json, "sealed_objects");

    // If "sealed_objects" exists but is not an array, remove it
    if (all_objs_json != NULL) {
        cJSON_DeleteItemFromObjectCaseSensitive(file_json, "sealed_objects");
    }

    // Create a new array
    cJSON *new_array = cJSON_CreateArray();
    if (new_array == NULL) {
        // Memory allocation failed
        return 1; // Error code
    }

    // Add the new array to the JSON object
    cJSON_AddItemToObject(file_json, "sealed_objects", new_array);

    // On success
    *out_all_objs_json = new_array;
    return 0;
}

/// @brief Delete all entries with the ID <machine_id_str>.
/// @param file_json 
/// @param machine_id_str 
/// @param machine_id_str_len 
/// @return 
static int _sealed_objs_file_delete_machine(cJSON* file_json, char* machine_id_str, size_t machine_id_str_len)
{
    // JSON format: 
    // {
    //     "sealed_objects": [
    //         {
    //             "machine_id": "[Base64 of the unique ID of machine 1]",
    //             "sealed_data": "[Base64 of the sealed data of machine 1]"
    //         },
    //         {
    //             "machine_id": "[Base64 of the unique ID of machine 2]",
    //             "sealed_data": "[Base64 of the sealed data of machine 2]"
    //         }
    //     ]
    // }

    int ret = 0;
    cJSON *all_objs_json = NULL;

    all_objs_json = cJSON_GetObjectItemCaseSensitive(file_json, "sealed_objects");
    if (!cJSON_IsArray(all_objs_json)) 
    {
        // Corrupted JSON structure <file_json>, clear the structure.
        ret = _sealed_objs_file_create_empty_json_root(file_json, &all_objs_json);
        if (ret)
            return 1;

        // It is a success, because the result <file_json> does not contain the machine ID.
        return 0;
    }

    // Get the array size
    int array_size = cJSON_GetArraySize(all_objs_json);

    // Iterate backwards to allow safe deletion of found JSON entries.
    for (int index = array_size - 1; index >= 0; index--)
    {
        cJSON *cur_obj_json = cJSON_GetArrayItem(all_objs_json, index);
        
        if (cJSON_IsObject(cur_obj_json))
        {
            cJSON* machine_id_base64_json = cJSON_GetObjectItemCaseSensitive(cur_obj_json, "machine_id");
            
            if (machine_id_base64_json && cJSON_IsString(machine_id_base64_json) && (machine_id_base64_json->valuestring != NULL))
            {
                // Find the entry for the current machine
                if(!strncmp(machine_id_str, machine_id_base64_json->valuestring, machine_id_str_len))
                {
                    // Found the entry, delete it
                    cJSON_DeleteItemFromArray(all_objs_json, index);
                }
            } 
        }
    }

    // On success
    return 0;
}

static int _sealed_objs_file_append_machine(
    cJSON* file_json, 
    char* machine_id_str, 
    size_t machine_id_str_len, 
    char* sealed_obj_base64_str, 
    size_t sealed_obj_base64_str_len
)
{
    // JSON format: 
    // {
    //     "sealed_objects": [
    //         {
    //             "machine_id": "[Base64 of the unique ID of machine 1]",
    //             "sealed_data": "[Base64 of the sealed data of machine 1]"
    //         },
    //         // ... other objects ...
    //     ]
    // }

    int ret = 0;

    // Step 1: Retrieve the "sealed_objects" array from the JSON
    cJSON *all_objs_json = cJSON_GetObjectItemCaseSensitive(file_json, "sealed_objects");
    if (!cJSON_IsArray(all_objs_json)) 
    {
        ret = _sealed_objs_file_create_empty_json_root(file_json, &all_objs_json);
        if (ret)
            return 1;
    }

    // Step 2: Create a new JSON object for the sealed_object
    cJSON *new_sealed_obj = cJSON_CreateObject();
    if (new_sealed_obj == NULL)
    {
        // Memory allocation failed
        return 1; // Error: Memory allocation failure
    }

    // Step 3: Add "machine_id" to the new sealed_object
    // Ensure that the machine_id_str is null-terminated
    char machine_id_null_terminated[machine_id_str_len + 1];
    memcpy(machine_id_null_terminated, machine_id_str, machine_id_str_len);
    machine_id_null_terminated[machine_id_str_len] = '\0';

    if (!cJSON_AddStringToObject(new_sealed_obj, "machine_id", machine_id_null_terminated))
    {
        // Failed to add "machine_id"
        cJSON_Delete(new_sealed_obj);
        return 1; // Error: Failed to add "machine_id"
    }

    // Step 4: Add "sealed_data" to the new sealed_object
    // Ensure that the sealed_obj_base64_str is null-terminated
    char sealed_data_null_terminated[sealed_obj_base64_str_len + 1];
    memcpy(sealed_data_null_terminated, sealed_obj_base64_str, sealed_obj_base64_str_len);
    sealed_data_null_terminated[sealed_obj_base64_str_len] = '\0';

    if (!cJSON_AddStringToObject(new_sealed_obj, "sealed_data", sealed_data_null_terminated))
    {
        // Failed to add "sealed_data"
        cJSON_Delete(new_sealed_obj);
        return 1; // Error: Failed to add "sealed_data"
    }

    // Step 5: Append the new sealed_object to the "sealed_objects" array
    cJSON_AddItemToArray(all_objs_json, new_sealed_obj);

    // On success
    return 0;
}

#define SECBASE_MACHINE_ID_BASE64_LENGTH    (2 * SECBASE_MACHINE_ID_MAX_LENGTH)

/// @brief Save a sealed object to the JSON file <filepath>.
/// @param filepath 
/// @param sb_sealed_obj 
/// @param sb_sealed_obj_size 
/// @return 
static int _sealed_obj_persist_to_file(const char *filepath, NS_HC_RESP_TPM_SEAL_HYPAPP_SECRETS* sb_sealed_obj, size_t sb_sealed_obj_size)
{
    MHV_STATUS sb_status = MHV_SUCCESS;
    int ret = 0;
    cJSON* file_json = NULL;
    uint8_t machine_id[SECBASE_MACHINE_ID_MAX_LENGTH] = {0};
    char machine_id_base64[SECBASE_MACHINE_ID_BASE64_LENGTH] = {0};
    char* sealed_obj_base64_str = NULL;
// [TODO] 64 is a magic number to make sure sufficient space for the BASE64 encoding result.
#define SEALED_OBJ_BASE64_LENGTH(sealed_obj_raw_size)  (max(2 * sealed_obj_raw_size, 64))
    size_t sealed_obj_base64_str_size = SEALED_OBJ_BASE64_LENGTH(sb_sealed_obj_size);

    // Generate machine ID string as the base64 encoding of the machine ID got from SecBase.
    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_GET_UNIQUE_MACHINE_ID, (ulong_t)&machine_id);
    if (MHV_FAIL(sb_status))
    {
        TOOL_ERR("Persist sealed object to file: Hypercall (%u) error: %u!\n", NS_HC_GET_UNIQUE_MACHINE_ID, (uint32_t)sb_status);
        ret = 1;
        goto out;
    }

    // Generate the base64 encoding of the machine ID.
    ret = base64_encode(machine_id, SECBASE_MACHINE_ID_MAX_LENGTH, machine_id_base64);
    if(!ret || (ret >= SECBASE_MACHINE_ID_BASE64_LENGTH))
    {
        TOOL_ERR("Persist sealed object to file: Failed to generate the base64 of the machine ID!\n");
        ret = 1;
        goto out;
    }

    // Generate the base64 encoding of the sealed object.
    sealed_obj_base64_str = malloc(sealed_obj_base64_str_size);
    if(!sealed_obj_base64_str)
    {
        TOOL_ERR("Persist sealed object to file: Failed to allocate memory for the base64 of the sealed object!\n");
        ret = 1;
        goto out;
    }
    memset(sealed_obj_base64_str, 0, sealed_obj_base64_str_size);

    ret = base64_encode((uint8_t *)sb_sealed_obj, sb_sealed_obj_size, sealed_obj_base64_str);
    if(!ret || (ret >= sealed_obj_base64_str_size))
    {
        TOOL_ERR("Persist sealed object to file: Failed to generate the base64 of the sealed object!\n");
        ret = 1;
        goto out;
    }

    // Write the sealed object <sb_sealed_obj> to the file <filepath>
    ret = _json_file_read_all(filepath, true, &file_json);
    if(ret)
    {
        TOOL_ERR("Persist sealed object to file: Open sealed objects file error! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    size_t machine_id_base64_len = strnlen(machine_id_base64, SECBASE_MACHINE_ID_BASE64_LENGTH);
    size_t sealed_obj_base64_str_len = strnlen(sealed_obj_base64_str, sealed_obj_base64_str_size);

    ret = _sealed_objs_file_delete_machine(file_json, machine_id_base64, machine_id_base64_len);
    if(ret)
    {
        TOOL_ERR("Persist sealed object to file: Delete machine from the sealed objects file error! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    ret = _sealed_objs_file_append_machine(file_json, machine_id_base64, machine_id_base64_len, sealed_obj_base64_str, sealed_obj_base64_str_len);
    if(ret)
    {
        TOOL_ERR("Persist sealed object to file: Append machine to the sealed objects file error! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    ret = _json_file_overwrite(filepath, file_json);
    if(ret)
    {
        TOOL_ERR("Persist sealed object to file: Write sealed objects file error! ret:%d\n", ret);
        ret = 1;
        goto out;
    }

    // On success
    ret = 0;

out:
    if(sealed_obj_base64_str)
        free(sealed_obj_base64_str);
    return ret;
}

/************ Public Functions **********/
int cmd_secbase_secrets_seal(const char *filepath)
{
    MHV_STATUS sb_status = MHV_SUCCESS;
    int ret = 0;
    NS_HC_RESP_TPM_SEAL_HYPAPP_SECRETS sb_sealed_obj = {0};
    size_t sb_sealed_obj_size = 0;

    // Check: parameters must be valid
    if (!filepath)
        return BAD_FUNC_ARG;

    // [NOTE] No need to check <tpm2_is_initialized()>, because it asks SecBase to perform the entire seal operation,
    // not the TPM library.

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
        return TPM_RC_FAILURE;

    // Get the seal result
    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_TPM_SEAL_HYPAPP_SECRETS, (ulong_t)&sb_sealed_obj);
    if (MHV_FAIL(sb_status))
    {
        TOOL_ERR("<cmd_secbase_secrets_seal> Hypercall (%u) error: %u!\n", NS_HC_TPM_SEAL_HYPAPP_SECRETS, (uint32_t)sb_status);
        return TPM_RC_FAILURE;
    }

    sb_sealed_obj_size = sizeof(sb_sealed_obj.seal_priv_size) + sizeof(sb_sealed_obj.seal_pub_size) + sb_sealed_obj.seal_priv_size + sb_sealed_obj.seal_pub_size;
    sb_sealed_obj_size = min(sb_sealed_obj_size, sizeof(NS_HC_RESP_TPM_SEAL_HYPAPP_SECRETS));

    // Replace the sealed object in the JSON file <filepath>
    ret = _sealed_obj_persist_to_file(filepath, &sb_sealed_obj, sb_sealed_obj_size);
    if(ret)
    {
        TOOL_ERR("<cmd_secbase_secrets_seal> Open sealed objects file error! ret:%d\n", ret);
        return TPM_RC_FAILURE;
    }

    // On success
    return TPM_RC_SUCCESS;
}

int cmd_secbase_secrets_remove(const char *filepath)
{
    MHV_STATUS sb_status = MHV_SUCCESS;
    int ret = 0;
    cJSON* file_json = NULL;
    uint8_t machine_id[SECBASE_MACHINE_ID_MAX_LENGTH] = {0};
    char machine_id_base64[SECBASE_MACHINE_ID_BASE64_LENGTH] = {0};

    // Check: parameters must be valid
    if (!filepath)
        return BAD_FUNC_ARG;

    // Generate machine ID string as the base64 encoding of the machine ID got from SecBase.
    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_GET_UNIQUE_MACHINE_ID, (ulong_t)&machine_id);
    if (MHV_FAIL(sb_status))
    {
        TOOL_ERR("Remove sealed object from file: Hypercall (%u) error: %u!\n", NS_HC_GET_UNIQUE_MACHINE_ID, (uint32_t)sb_status);
        return TPM_RC_FAILURE;
    }

    // Generate the base64 encoding of the machine ID.
    ret = base64_encode(machine_id, SECBASE_MACHINE_ID_MAX_LENGTH, machine_id_base64);
    if(!ret || (ret >= SECBASE_MACHINE_ID_BASE64_LENGTH))
    {
        TOOL_ERR("Remove sealed object from file: Failed to generate the base64 of the machine ID!\n");
        return TPM_RC_FAILURE;
    }

    // Remove the machine and its sealed object from the file <filepath>.
    ret = _json_file_read_all(filepath, true, &file_json);
    if(ret)
    {
        TOOL_ERR("Remove sealed object from file: Open sealed objects file error! ret:%d\n", ret);
        return TPM_RC_FAILURE;
    }

    size_t machine_id_base64_len = strnlen(machine_id_base64, SECBASE_MACHINE_ID_BASE64_LENGTH);

    ret = _sealed_objs_file_delete_machine(file_json, machine_id_base64, machine_id_base64_len);
    if(ret)
    {
        TOOL_ERR("Remove sealed object from file: Delete machine from the sealed objects file error! ret:%d\n", ret);
        return TPM_RC_FAILURE;
    }

    ret = _json_file_overwrite(filepath, file_json);
    if(ret)
    {
        TOOL_ERR("Remove sealed object from file: Write sealed objects file error! ret:%d\n", ret);
        return TPM_RC_FAILURE;
    }

    // On success
    return TPM_RC_SUCCESS;
}
