// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sap_headers.h>
#include <palddk.h>

#include <sap_test.h>

/// @brief SecProcess constant properties (e.g., name, type).
const SP_CONST_PARAMETERS sp_constant_properties __attribute__((section(".version_rodata"))) = {
	.version_number = GREENBOX_VERSION_NUMBER,	// SecProcess version number

	.sp_type = SP_APPLICATION,			// SecProcess type
	"Test-SecApp",					// SecProcess name

	.is_trusted = false,				// Is the SecProcess a trusted one?
	.sap_params = 
	{
		._reserved = 0					// Is the SecProcess a INIT SecProcess?
	}
};

struct test_info
{
	char* name;
	bool (*test_fn)(void);
};

#define NUM_TESTS	100

struct test_info testcases[NUM_TESTS] = 
{
	{"test_netdb_gethostbyname", test_netdb_gethostbyname},
	{"test_netdb_gethostbyname_localhost", test_netdb_gethostbyname_localhost},
	{"test_netdb_hostname_to_ip", test_netdb_hostname_to_ip},

	{"test_socket_send_recv", test_socket_send_recv},
	
	{"test_file_create_write", test_file_create_write},
	{"test_file_create_write_longdata", test_file_create_write_longdata},
	{"test_file_open_read", test_file_open_read},
	{"test_file_open_read_longdata", test_file_open_read_longdata},
	{"test_file_open_read_more_longdata", test_file_open_read_more_longdata},

	{"test_timer_sleep_long", test_timer_sleep_long},
    
    // Openssl
    {"test_openssl_generate_rand_number", test_openssl_generate_rand_number},
    {"test_openssl_get_hw_prng", test_openssl_get_hw_prng},
    
	{NULL, NULL}
};


void sp_main(SP_UTASK* params)
{
	uint32_t total_pass = 0, i = 0;
	bool status = false;
	
	for(i = 0; i < NUM_TESTS; i++)
	{
		if(!testcases[i].test_fn)
			break;
		
		status = testcases[i].test_fn();
		if(!status)
			TEST_FAIL("%s\n", testcases[i].name);
		else
        {
            TEST_PASS("%s\n", testcases[i].name);
			total_pass++;
        }
	}

	printf("Total Pass: %u\n", total_pass);
}

void sp_exit(SP_UTASK* sp_params)
{

}

void sp_handle_intr(intr_t vector)
{
	// Empty function
}

void sp_handle_sp_request(SP_MESSAGE* msg)
{

}

bool sp_handle_app_request(struct ipc_msg* msg)
{
	// Sanity checks
	if(!msg)
		return false;

	// Process events
	return true;
}

void sp_handle_exception(void)
{
	sp_exit(current_sp);
}