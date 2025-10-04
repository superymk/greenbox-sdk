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

#include <ipc/sl_rpc/sl_rpc_protocol.h>
#include <secloader_status.h>
#include "sl_gui.h"

// Forward declaration
class SOSLoader_GUIFrm_Main;

// Class declaration
#ifdef __WINDOWS__
#include <winsock2.h>
#endif

#include <cJSON.h>

#ifdef __LINUX__
    // define some symbols which winsock.h defines but traditional BSD headers
    // don't
    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET (-1)
    #endif
#endif // __LINUX__

/// Thread-safe in C++11, not safe in C++03
class SOSLoader_GUI_RPC
{
	public:
		static SOSLoader_GUI_RPC& getInstance()
		{
			static SOSLoader_GUI_RPC instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.

			return instance;
		}

		SL_STATUS call_sl_init(void);
		SL_STATUS call_sl_fini(void);
		SL_STATUS call_sl_load_and_run_secos(SOSLoader_GUIFrm_Main* in_frm_main);
		SL_STATUS call_sl_terminate_and_unregister_secos(void);
		SL_STATUS call_sl_dump_dmesg(SOSLoader_GUIFrm_Main* in_frm_main);

		SL_STATUS call_sl_secprocesses_soft_terminate(spid_t sp_id);
		SL_STATUS call_sl_secprocesses_get_ids_and_names(struct sp_info_id_name_resp *resp_buf);
		SL_STATUS call_sl_get_dtm_status(uint64_t* out_status);
		// SL_STATUS call_sl_dev_usb_hc_reset(void);

	private:
		#ifdef __WINDOWS__
			SOCKET ConnectSocket = INVALID_SOCKET;
		#elif defined (__LINUX__)
			int ConnectSocket = INVALID_SOCKET;
		#endif // __WINDOWS__

		SOSLoader_GUI_RPC();

        ~SOSLoader_GUI_RPC();

		// C++ 03
        // ========
        // Don't forget to declare these two. You want to make sure they
        // are inaccessible(especially from outside), otherwise, you may accidentally get copies of
        // your singleton appearing.
        // SOSLoader_GUI_RPC(SOSLoader_GUI_RPC const&);              // Don't Implement
        // void operator=(SOSLoader_GUI_RPC const&); // Don't implement

		// C++ 11
        // =======
        // We can use the better technique of deleting the methods
        // we don't want.

		// ~SOSLoader_GUI_RPC();
		int init_socket();
		void close_socket();
		bool is_socket_valid();

		int send_cmd(uint32_t cmd, uint32_t arg1, uint32_t arg2, uint32_t arg3);


		int socket_send_str(char *str);
		int socket_recv_str(char *out_str, void (*func_wait)(void));
		int parse_resp(const char *resp_str, SL_STATUS *out_status, cJSON **out_root_json, cJSON **out_resp_data_json);

	public:
        SOSLoader_GUI_RPC(SOSLoader_GUI_RPC const&)     = delete;
        void operator=(SOSLoader_GUI_RPC const&)  		= delete;

        // Note: Scott Meyers mentions in his Effective Modern
        //       C++ book, that deleted functions should generally
        //       be public as it results in better error messages
        //       due to the compilers behavior to check accessibility
        //       before deleted status
};