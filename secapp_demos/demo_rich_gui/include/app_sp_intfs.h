// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef APP_SP_INTFS_H
#define APP_SP_INTFS_H

#ifndef __ASSEMBLY__


/// @brief Configurations of trusted path devices
struct sap_window_conf
{
    uint32_t win_logical_width;
    uint32_t win_logical_height;
    float win_scale;
};

typedef struct 
{
    struct sap_window_conf window_info;
} SAP_CONFIG;

#endif // __ASSEMBLY__
#endif // APP_SP_INTFS_H