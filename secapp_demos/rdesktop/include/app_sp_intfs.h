// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SP_INTFS_H
#define APP_SP_INTFS_H

#ifndef __ASSEMBLY__

#define URL_LENGTH_MAX      (2083)
#define PORT_NUMBER_LENGTH_MAX      (5)
#define SAP_LOGON_URL_LENGTH_MAX    (URL_LENGTH_MAX + 1 + PORT_NUMBER_LENGTH_MAX) // Add length by 1 for the colon; e.g., 192.168.137.1:8000.

/// @brief Configurations of trusted path devices
struct sap_window_conf
{
    uint32_t win_logical_width;
    uint32_t win_logical_height;
    float win_scale;
};

struct sap_logon_conf
{
    char url[SAP_LOGON_URL_LENGTH_MAX + 1]; // One additional char for the ending '\0'
};

struct sap_display_conf
{
    uint8_t compress;
    uint8_t quality;
};

typedef struct 
{
    struct sap_window_conf window_info;
    struct sap_logon_conf logon_info;
    struct sap_display_conf display_info;


} SAP_CONFIG;

#endif // __ASSEMBLY__
#endif // APP_SP_INTFS_H