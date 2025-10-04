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

#include <chrono>
#include <thread>

#include "sl_gui_rpc.h"
#include "SOSLoader_GUIFrm_Main.h"

#ifdef __WINDOWS__
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#elif defined(__LINUX__)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#endif // __WINDOWS__

using std::string;

#define DEFAULT_BUFLEN 4096

#ifdef __WINDOWS__
int SOSLoader_GUI_RPC::init_socket()
{
    WSADATA wsaData;

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return -1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("localhost", SL_SERVER_PORT_STR, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        this->close_socket();
        return -1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            this->close_socket();
            return -1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        // Set the socket to non-blocking mode.
        u_long iMode = 1;
        iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
        if (iResult == SOCKET_ERROR)
        {
            printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
            this->close_socket();
            return -1;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        this->close_socket();
        return -1;
    }

    return 0;
}

void SOSLoader_GUI_RPC::close_socket()
{
    if (this->ConnectSocket != INVALID_SOCKET)
    {
        closesocket(this->ConnectSocket);
        this->ConnectSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

int SOSLoader_GUI_RPC::socket_send_str(char *str)
{
    int iResult;

    iResult = send(this->ConnectSocket, str, (int)strnlen(str, MAX_JSON_MSG_SIZE), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        this->close_socket();
        return -1;
    }

    return 0;
}

int SOSLoader_GUI_RPC::socket_recv_str(char *out_str, void (*func_wait)(void))
{
    int iResult;

    // Check: <out_str> must be valid
    if (!out_str)
        return -1;

    while (true)
    {
        // Check the receive status.
        iResult = recv(this->ConnectSocket, out_str, MAX_JSON_MSG_SIZE, 0);
        if (iResult == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                // The socket is not ready to receive yet.
                if (func_wait)
                    func_wait();
                continue;
            }
            else
            {
                // An error occurred.
                printf("recv failed with error: %d\n", WSAGetLastError());
                this->close_socket();
                return -1;
            }
        }
        else if (iResult == 0)
        {
            // The connection was closed by the server.
            printf("The connection was closed.\n");
            this->close_socket();
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

#elif defined(__LINUX__)

int SOSLoader_GUI_RPC::init_socket()
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
    this->close_socket();
    return -1;
}

void SOSLoader_GUI_RPC::close_socket()
{
    if ((this->ConnectSocket != 0) && (this->ConnectSocket != INVALID_SOCKET))
    {
        close(this->ConnectSocket);
        this->ConnectSocket = INVALID_SOCKET;
    }
}

int SOSLoader_GUI_RPC::socket_send_str(char *str)
{
    int send_status;

    send_status = send(this->ConnectSocket, str, (int)strnlen(str, MAX_JSON_MSG_SIZE), 0);
    if (send_status == -1)
    {
        printf("send failed with error: %s (%d)\n", strerror(errno), errno);
        this->close_socket();
        return -1;
    }

    return 0;
}

int SOSLoader_GUI_RPC::socket_recv_str(char *out_str, void (*func_wait)(void))
{
    int iResult;

    // Check: <out_str> must be valid
    if (!out_str)
        return -1;

    while (true)
    {
        // Check the receive status.
        iResult = recv(this->ConnectSocket, out_str, MAX_JSON_MSG_SIZE, 0);
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
                this->close_socket();
                return -1;
            }
        }
        else if (iResult == 0)
        {
            // The connection was closed by the server.
            printf("The connection was closed.\n");
            this->close_socket();
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

#endif // __WINDOWS__

SOSLoader_GUI_RPC::SOSLoader_GUI_RPC()
{
    int ret = 0;
    ret = init_socket();
    if (ret)
    {
        printf("Init socket error!\n");
    }
}

SOSLoader_GUI_RPC::~SOSLoader_GUI_RPC()
{
    close_socket();
}

int SOSLoader_GUI_RPC::send_cmd(uint32_t cmd, uint32_t arg1, uint32_t arg2, uint32_t arg3)
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

int SOSLoader_GUI_RPC::parse_resp(const char *resp_str, SL_STATUS *out_status, cJSON **out_root_json, cJSON **out_resp_data_json)
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

SL_STATUS SOSLoader_GUI_RPC::call_sl_init(void)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];

    ret = this->send_cmd(SL_SERVER_CMD_sl_init, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_init error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_init response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, NULL);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_init response error!\n");
        return SL_EINTERNAL;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_fini(void)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];

    ret = this->send_cmd(SL_SERVER_CMD_sl_fini, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_fini error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_fini response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, NULL);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_fini response error!\n");
        return SL_EINTERNAL;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_load_and_run_secos(SOSLoader_GUIFrm_Main* in_frm_main)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];
    static SOSLoader_GUIFrm_Main* frm_main = in_frm_main;

    ret = this->send_cmd(SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, []
                                {
                                    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    // frm_main->ShowSOSLoaderProgress();
                                    ::wxSafeYield();
                                }
          );
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, NULL);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_load_and_run_secos_and_secsrvs response error!\n");
        return SL_EINTERNAL;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_terminate_and_unregister_secos(void)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];

    ret = this->send_cmd(SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, []
                                { ::wxSafeYield(); });
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, NULL);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_terminate_and_unload_secos_and_secsrvs response error!\n");
        return SL_EINTERNAL;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_dump_dmesg(SOSLoader_GUIFrm_Main* in_frm_main)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];
    static SOSLoader_GUIFrm_Main* frm_main = in_frm_main;

    ret = this->send_cmd(SL_SERVER_CMD_sl_dump_dmesg, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_dump_dmesg error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, []
                                {
                                    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    // frm_main->ShowSOSLoaderProgress();
                                    ::wxSafeYield();
                                }
          );
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_dump_dmesg response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, NULL);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_dump_dmesg response error!\n");
        return SL_EINTERNAL;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_secprocesses_soft_terminate(spid_t sp_id)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];

    ret = this->send_cmd(SL_SERVER_CMD_sl_secprocesses_soft_terminate, (uint32_t)sp_id, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_secprocesses_soft_terminate error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, []
                                { ::wxYield(); }); // This RPC operation is fast (i.e., takes a few milliseconds), so we can use <wxYield> instead of <wxSafeYield>.
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_secprocesses_soft_terminate response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, NULL);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_secprocesses_soft_terminate response error!\n");
        return SL_EINTERNAL;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_secprocesses_get_ids_and_names(struct sp_info_id_name_resp *resp_buf)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];
    cJSON *resp_data_json = NULL;

    // Check: <resp_buf> must be valid
    if (!resp_buf)
        return SL_EINVAL;

    ret = this->send_cmd(SL_SERVER_CMD_sl_secprocesses_get_ids_and_names, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_secprocesses_get_ids_and_names error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, []
                                { ::wxYield(); }); // This RPC operation is fast (i.e., takes a few milliseconds), so we can use <wxYield> instead of <wxSafeYield>.
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_secprocesses_get_ids_and_names response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, &resp_data_json);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_secprocesses_get_ids_and_names response error!\n");
        return SL_EINTERNAL;
    }

    // Fill <resp_buf>
    {
        cJSON *info_json = NULL;
        cJSON *infos_json = NULL;
        infos_json = cJSON_GetObjectItemCaseSensitive(resp_data_json, "info");
        uint32_t i = 0;

        cJSON_ArrayForEach(info_json, infos_json)
        {
            cJSON *sp_id_json = cJSON_GetObjectItemCaseSensitive(info_json, "sp_id");
            cJSON *sp_name_json = cJSON_GetObjectItemCaseSensitive(info_json, "sp_name");

            if (cJSON_IsNumber(sp_id_json))
                resp_buf->info[i].sp_id = (spid_t)sp_id_json->valuedouble;
            else
                resp_buf->info[i].sp_id = 0;

            if (cJSON_IsString(sp_name_json) && sp_name_json->valuestring != NULL)
            {
                char *sp_name = sp_name_json->valuestring;
                strncpy(resp_buf->info[i].sp_name, sp_name, SP_NAME_MAX_SIZE);
            }
            else
            {
                memset(resp_buf->info[i].sp_name, 0, SP_NAME_MAX_SIZE);
            }

            i++;
        }
        resp_buf->num_info = i;
    }

    return result_status;
}

SL_STATUS SOSLoader_GUI_RPC::call_sl_get_dtm_status(uint64_t* out_status)
{
    SL_STATUS result_status;
    int ret = 0;
    char resp_str[MAX_JSON_MSG_SIZE];
    cJSON *resp_data_json = NULL;

    // Check: <out_status> must be valid
    if (!out_status)
        return SL_EINVAL;

    ret = this->send_cmd(SL_SERVER_CMD_sl_get_dtm_status, 0, 0, 0);
    if (ret)
    {
        printf("Request SL_SERVER_CMD_sl_get_dtm_status error!\n");
        return SL_EINTERNAL;
    }

    ret = this->socket_recv_str(resp_str, NULL);
    if (ret)
    {
        printf("Receive SL_SERVER_CMD_sl_get_dtm_status response error!\n");
        return SL_EINTERNAL;
    }

    ret = this->parse_resp(resp_str, &result_status, &resp_data_json);
    if (ret)
    {
        printf("Parse SL_SERVER_CMD_sl_get_dtm_status response error!\n");
        return SL_EINTERNAL;
    }

    // Return <out_status>
    {
        cJSON *retval1_json = NULL;
        retval1_json = cJSON_GetObjectItemCaseSensitive(resp_data_json, "ret_val1");

        if (cJSON_IsNumber(retval1_json))
            *out_status = (uint64_t)retval1_json->valuedouble;
        else
            *out_status = 0;
    }

    return result_status;
}

// SL_STATUS SOSLoader_GUI_RPC::call_sl_dev_usb_hc_reset(void)
// {
//     SL_STATUS result_status;
//     int ret = 0;
//     char resp_str[MAX_JSON_MSG_SIZE];

//     ret = this->send_cmd(SL_SERVER_CMD_sl_dev_usb_hc_reset, 0, 0, 0);
//     if (ret)
//     {
//         printf("Request SL_SERVER_CMD_sl_dev_usb_hc_reset error!\n");
//         return SL_EINTERNAL;
//     }

//     ret = this->socket_recv_str(resp_str, []
//                                 {
//                                     // std::this_thread::sleep_for(std::chrono::milliseconds(500));
//                                     // frm_main->ShowSOSLoaderProgress();
//                                     ::wxSafeYield();
//                                 }
//     if (ret)
//     {
//         printf("Receive SL_SERVER_CMD_sl_dev_usb_hc_reset response error!\n");
//         return SL_EINTERNAL;
//     }

//     ret = this->parse_resp(resp_str, &result_status, NULL);
//     if (ret)
//     {
//         printf("Parse SL_SERVER_CMD_sl_dev_usb_hc_reset response error!\n");
//         return SL_EINTERNAL;
//     }

//     return result_status;
// }