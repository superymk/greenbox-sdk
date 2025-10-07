// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

/// @file RPC Client to communicate with SOSLoader server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <appddk.h>
#include <secloader_status.h>
#include "../cJSON/cJSON.h"

#define DEFAULT_BUFLEN 4096

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET (-1)
#endif

static void close_socket(void);


static int ConnectSocket = INVALID_SOCKET;


static int init_socket()
{
    int connection_status = 0;
    int flags = 0;

    ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ConnectSocket == -1)
    {
        printf("socket init error\n");
        goto err;
    }

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SL_SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    connection_status = connect(ConnectSocket, (struct sockaddr *)&server_address, sizeof(server_address));
    // check for connection_status
    if (connection_status == -1)
    {
        printf("The connection has error\n");
        goto err;
    }

    // Set the socket to non-blocking mode
    flags = fcntl(ConnectSocket, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        goto err;
    }
    if (fcntl(ConnectSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL");
        goto err;
    }

    return 0;

err:
    close_socket();
    return -1;
}

static void close_socket(void)
{
    if ((ConnectSocket != 0) && (ConnectSocket != INVALID_SOCKET))
    {
        close(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }
}

static int socket_send_str(char *str)
{
    int send_status;

    send_status = send(ConnectSocket, str, (int)strnlen(str, MAX_JSON_MSG_SIZE), 0);
    if (send_status == -1)
    {
        printf("send failed with error: %s (%d)\n", strerror(errno), errno);
        close_socket();
        return -1;
    }

    return 0;
}

static int socket_recv_str(char *out_str, void (*func_wait)(void))
{
    int iResult;

    // Check: <out_str> must be valid
    if (!out_str)
        return -1;

    while (true)
    {
        // Check the receive status.
        iResult = recv(ConnectSocket, out_str, MAX_JSON_MSG_SIZE, 0);
        if (iResult == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // The socket is not ready to receive yet.
                if (func_wait)
                    func_wait();
                continue;
            }
            else
            {
                // An error occurred.
                printf("recv failed with error: %d\n", errno);
                close_socket();
                return -1;
            }
        }
        else if (iResult == 0)
        {
            // The connection was closed by the server.
            printf("The connection was closed.\n");
            close_socket();
            return 1;
        }
        else
        {
            // Data was received.
            // printf("Received %d bytes: %s\n", iResult, recvBuffer);
            return 0;
        }
    }

    // Should not hit
    return -1;
}

/// @brief Send a request to SOSLoader server.
/// @param cmd 
/// @param arg1 
/// @param arg2 
/// @param arg3 
/// @return 
__attribute__((unused)) static int send_cmd(uint32_t cmd, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    int status = 0;
    cJSON *req_json = NULL;
    cJSON *node_json = NULL;
    char *req_str = NULL;

    req_json = cJSON_CreateObject();
    if (!req_json)
        goto err;

    // Serialize command
    node_json = cJSON_CreateNumber(cmd);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "cmd", node_json);

    // Serialize args
    node_json = cJSON_CreateNumber(arg1);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "arg1", node_json);

    node_json = cJSON_CreateNumber(arg2);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "arg2", node_json);

    node_json = cJSON_CreateNumber(arg3);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "arg3", node_json);

    // Send the request
    req_str = cJSON_Print(req_json);
    if(!req_str)
        goto err;

    status = socket_send_str(req_str);
    cJSON_free(req_str);
    if (status)
        goto err;

    // On success
    return 0;

err:
    printf("Send command failed: Failed to construct request packet\n");
    if (req_json)
        cJSON_Delete(req_json);
    return -1;
}

__attribute__((unused)) static int send_cmd_2json_args(
    uint32_t cmd, 
    uint32_t arg1, 
    uint32_t arg2, 
    uint32_t arg3, 
    const char* arg1_json_str, 
    cJSON *arg1_json, 
    const char* arg2_json_str, 
    cJSON *arg2_json
)
{
    int status = 0;
    cJSON *req_json = NULL;
    cJSON *node_json = NULL;
    char *req_str = NULL;

    req_json = cJSON_CreateObject();
    if (!req_json)
        goto err;

    // Serialize command
    node_json = cJSON_CreateNumber(cmd);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "cmd", node_json);

    // Serialize args
    node_json = cJSON_CreateNumber(arg1);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "arg1", node_json);

    node_json = cJSON_CreateNumber(arg2);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "arg2", node_json);

    node_json = cJSON_CreateNumber(arg3);
    if (node_json == NULL)
        goto err;
    cJSON_AddItemToObject(req_json, "arg3", node_json);

    if(arg1_json_str && arg1_json)
    {
        cJSON_AddItemToObject(req_json, arg1_json_str, arg1_json);
    }

    if(arg2_json_str && arg2_json)
    {
        cJSON_AddItemToObject(req_json, arg2_json_str, arg2_json);
    }

    // Send the request
    req_str = cJSON_Print(req_json);
    if(!req_str)
        goto err;
        
    status = socket_send_str(req_str);
    cJSON_free(req_str);
    if (status)
        goto err;

    // On success
    return 0;

err:
    printf("Send command failed: Failed to construct request packet\n");
    if (req_json)
        cJSON_Delete(req_json);
    return -1;
}

__attribute__((unused)) static int send_cmd_1json_arg(uint32_t cmd, uint32_t arg1, uint32_t arg2, uint32_t arg3, const char* arg1_json_str, cJSON *arg1_json)
{
    return send_cmd_2json_args(cmd, arg1, arg2, arg3, arg1_json_str, arg1_json, NULL, NULL);
}

static int parse_resp(const char *resp_str, SL_STATUS *out_status, cJSON **out_root_json, cJSON **out_resp_data_json)
{
    cJSON *resp_json = NULL;
    cJSON *status_json = NULL;
    cJSON *resp_data_json = NULL;

    if (!resp_str)
        return -1;

    resp_json = cJSON_Parse(resp_str);
    if (!resp_json)
        return -1;

    // Parse status
    status_json = cJSON_GetObjectItemCaseSensitive(resp_json, "status");
    if (out_status)
    {
        if (cJSON_IsNumber(status_json))
            *out_status = (SL_STATUS)status_json->valuedouble;
        else
            *out_status = SL_EINTERNAL;
    }

    // Parse resp_data
    resp_data_json = cJSON_GetObjectItemCaseSensitive(resp_json, "resp_data");
    if (out_resp_data_json)
        *out_resp_data_json = resp_data_json;

    *out_root_json = resp_json;                // keep ownership in caller
    return 0;
}




/********* Public Functions *********/
int app_sl_rpc_init(void)
{
    int ret = 0;
    
    // Return success if the connection has already been established
    if(app_sl_rpc_is_initialized())
    {
        // Return success
        return 0;
    }

    ret = init_socket();
    if (ret)
    {
        printf("Init socket error!\n");
        return ret;
    }

    // On success
    return 0;
}

void app_sl_rpc_fini(void)
{
    close_socket();
}

bool app_sl_rpc_is_initialized(void)
{
    return (ConnectSocket != INVALID_SOCKET) ? true : false;
}

int app_sl_rpc_call_sl_mark_tasks_after_secos_unload(
    uint32_t task_cmd,
    SL_STATUS* out_sl_status
)
{
    int ret = 0;
    char* resp_str = NULL;
    cJSON *root_json = NULL;

    // Check: params must be valid
    if(!out_sl_status)
        return -1;
    if(!app_sl_rpc_is_initialized())
        return -1;

    resp_str = (char*)malloc(MAX_JSON_MSG_SIZE);
    if (!resp_str)
    {
        ret = -1;
        goto out;
    }
    memset(resp_str, 0, MAX_JSON_MSG_SIZE);

    // Send a request to SOSLoader server
    ret = send_cmd(SL_SERVER_CMD_sl_mark_tasks_after_secos_unload, task_cmd, 0, 0);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_sl_mark_tasks_after_secos_unload> Request SOSLoader server error!\n");
        ret = -1;
        goto out;
    }

    ret = socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_sl_mark_tasks_after_secos_unload> Receive response error!\n");
        ret = -1;
        goto out;
    }

    ret = parse_resp(resp_str, out_sl_status, &root_json, NULL);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_sl_mark_tasks_after_secos_unload> Parse response error!\n");
        ret = -1;
        goto out;
    }

    // On success
    ret = 0;

out:
    if (root_json)
        cJSON_Delete(root_json); // free JSON tree after use
    if(resp_str)
        free(resp_str);

    return ret;
}

int app_sl_rpc_call_sl_secbase_tpm_get_quote(
    char* in_quote_nonce_str, 
    uint64_t in_quote_nonce_timestamp, 
    uint32_t pcr_mhv_img, 
    uint32_t pcr_mhv_boot_state,
    SL_STATUS* out_sl_status, 
    char* out_tpm_quote_files_dirname
)
{
    int ret = 0;
    char* resp_str = NULL;
    const char* arg1_json_str = "quote_nonce_str";
    const char* arg2_json_str = "quote_nonce_timestamp";
    cJSON *arg1_json = NULL;
    cJSON *arg2_json = NULL;
    char quote_nonce_str[TPM_QUOTE_NONCE_STRING_MAX_LENGTH + 1] = {0}; // + 1 for the string terminator.
    cJSON *resp_data_json = NULL;
    cJSON *root_json = NULL;

    // Check: params must be valid
    if(!in_quote_nonce_str || !out_sl_status)
        return -1;
    if(!app_sl_rpc_is_initialized())
        return -1;

    resp_str = (char*)malloc(MAX_JSON_MSG_SIZE);
    if (!resp_str)
    {
        ret = -1;
        goto out;
    }
    memset(resp_str, 0, MAX_JSON_MSG_SIZE);

    strncpy(quote_nonce_str, in_quote_nonce_str, TPM_QUOTE_NONCE_STRING_MAX_LENGTH);
    arg1_json = cJSON_CreateString(quote_nonce_str);
    if(!arg1_json)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_quote> Create JSON of TPM quote string error!\n");
        ret = -1;
        goto out;
    }

    arg2_json = cJSON_CreateNumber(in_quote_nonce_timestamp);
    if(!arg2_json)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_quote> Create JSON of TPM quote timestamp error!\n");
        ret = -1;
        goto out;
    }

    // Send a request to SOSLoader server
    ret = send_cmd_2json_args(SL_SERVER_CMD_secbase_tpm_get_quote, pcr_mhv_img, pcr_mhv_boot_state, 0, arg1_json_str, arg1_json, arg2_json_str, arg2_json);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_quote> Request SOSLoader server error!\n");
        ret = -1;
        goto out;
    }

    ret = socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_quote> Receive response error!\n");
        ret = -1;
        goto out;
    }

    ret = parse_resp(resp_str, out_sl_status, &root_json, &resp_data_json);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_quote> Parse response error!\n");
        ret = -1;
        goto out;
    }

    // Return <out_tpm_quote_files_dirname> 
    {
        cJSON *retval1_json = NULL;
        retval1_json = cJSON_GetObjectItemCaseSensitive(resp_data_json, "ret_tpm_quote_files_path");

        if (cJSON_IsString(retval1_json))
        {
            if(out_tpm_quote_files_dirname)
                strncpy(out_tpm_quote_files_dirname, retval1_json->valuestring, FS_MAX_PATH_BUFFER_LEN);
        }
    }

    // On success
    ret = 0;

out:
    if (root_json)
        cJSON_Delete(root_json); // free JSON tree after use
    if(resp_str)
        free(resp_str);

    return ret;
}

int app_sl_rpc_call_sl_secbase_tpm_get_pcrs(
    uint32_t pcr_mhv_img, 
    uint32_t pcr_mhv_boot_state,
    SL_STATUS* out_sl_status, 
    char* out_tpm_pcrs_files_dirname
)
{
    int ret = 0;
    char* resp_str = NULL;
    cJSON *resp_data_json = NULL;
    cJSON *root_json = NULL;

    // Check: params must be valid
    if(!out_sl_status)
        return -1;
    if(!app_sl_rpc_is_initialized())
        return -1;

    resp_str = (char*)malloc(MAX_JSON_MSG_SIZE);
    if (!resp_str)
    {
        ret = -1;
        goto out;
    }
    memset(resp_str, 0, MAX_JSON_MSG_SIZE);

    // Send a request to SOSLoader server
    ret = send_cmd(SL_SERVER_CMD_secbase_tpm_get_pcrs, pcr_mhv_img, pcr_mhv_boot_state, 0);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_pcrs> Request SOSLoader server error!\n");
        ret = -1;
        goto out;
    }

    ret = socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_pcrs> Receive response error!\n");
        ret = -1;
        goto out;
    }

    ret = parse_resp(resp_str, out_sl_status, &root_json, &resp_data_json);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_secbase_tpm_get_pcrs> Parse response error!\n");
        ret = -1;
        goto out;
    }

    // Return <out_tpm_pcrs_files_dirname> 
    {
        cJSON *retval1_json = NULL;
        retval1_json = cJSON_GetObjectItemCaseSensitive(resp_data_json, "ret_tpm_pcrs_files_path");

        if (cJSON_IsString(retval1_json))
        {
            if(out_tpm_pcrs_files_dirname)
                strncpy(out_tpm_pcrs_files_dirname, retval1_json->valuestring, FS_MAX_PATH_BUFFER_LEN);
        }
    }

    // On success
    ret = 0;

out:
    if (root_json)
        cJSON_Delete(root_json); // free JSON tree after use
    if(resp_str)
        free(resp_str);

    return ret;
}

int app_sl_rpc_call_sl_os_efi_get_tpm2_event_logs_trace(
    uint32_t pcr_mhv_img, 
    SL_STATUS* out_sl_status, 
    char* out_evl_trace_file_dirname
)
{
    int ret = 0;
    char* resp_str = NULL;
    cJSON *resp_data_json = NULL;
    cJSON *root_json = NULL;

    // Check: params must be valid
    if(!out_sl_status)
        return -1;
    if(!app_sl_rpc_is_initialized())
        return -1;

    resp_str = (char*)malloc(MAX_JSON_MSG_SIZE);
    if (!resp_str)
    {
        ret = -1;
        goto out;
    }
    memset(resp_str, 0, MAX_JSON_MSG_SIZE);

    // Send a request to SOSLoader server
    ret = send_cmd(SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace, pcr_mhv_img, 0, 0);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace> Request SOSLoader server error!\n");
        ret = -1;
        goto out;
    }

    ret = socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace> Receive response error!\n");
        ret = -1;
        goto out;
    }

    ret = parse_resp(resp_str, out_sl_status, &root_json, &resp_data_json);
    if (ret)
    {
        printf("[App-SL RPC] <SL_SERVER_CMD_os_efi_get_tpm2_event_logs_trace> Parse response error!\n");
        ret = -1;
        goto out;
    }

    // Return <out_evl_trace_file_dirname> 
    {
        cJSON *retval1_json = NULL;
        retval1_json = cJSON_GetObjectItemCaseSensitive(resp_data_json, "ret_evl_trace_file_path");

        if (cJSON_IsString(retval1_json))
        {
            if(out_evl_trace_file_dirname)
                strncpy(out_evl_trace_file_dirname, retval1_json->valuestring, FS_MAX_PATH_BUFFER_LEN);
        }
    }

    // On success
    ret = 0;

out:
    if (root_json)
        cJSON_Delete(root_json); // free JSON tree after use
    if(resp_str)
        free(resp_str);

    return ret;
}