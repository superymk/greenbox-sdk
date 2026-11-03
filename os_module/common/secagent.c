// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <sa_headers.h>

static struct os_funcs os_funcs;
struct os_funcs *os = &os_funcs;
static bool is_sa_running = true;

void secagent_mark_running(void)
{
    is_sa_running = true;
}

void secagent_mark_not_running(void)
{
    is_sa_running = false;
}

bool secagent_is_running(void)
{
    return is_sa_running;
}

OSSTATUS secagent_init(void)
{
    OSSTATUS status = OS_SUCCESS;

    // Initialize required OS functions
    os_init();

    // Test the presence of Secbase
    if (!os->secbase_test_presence())
    {
        // Use <PRINT_ERR_FORCE_OS> instead of <DRV_ERR>. Otherwise, the SecAgent may crash due to sending hypercalls
        // to the non-exist mHV
        PRINT_ERR_FORCE_OS("SecBase is not running!\n");
        status = E_CHKERR;
        goto test_presence_err;
    }

    // Provision resources needed by SecOS
    DRV_INFO("Loading SecAgent\n");
    __x86_os_cpu_lapic_mapping_init();

    // Init SecAgent-App communication
    //// Init asynchronous event notifications
    os->os_app_notif_subsystem_init();
    //// Init event caching subsystem
    app_notif_mgr_init();

    // Init utils functions for the NS domain
    status = nsdom_utils_init();
    if (SA_FAIL(status))
    {
        DRV_ERR("Init NS domain utils functions error!\n");
        goto nsdom_utils_init_err;
    }

    // On success
    secagent_mark_running();
    DRV_INFO("SecAgent Initialization Succeed!\n");
    return OS_SUCCESS;

nsdom_utils_init_err:
test_presence_err:
    os_fini();
    secagent_mark_not_running();
    return status;
}

void secagent_fini(void)
{
    DRV_INFO("SecAgent is exiting.\n");
    secagent_mark_not_running();

    // Finalize utils functions for the NS domain
    nsdom_utils_fini();

#ifdef WINNT
    // [TODO][Issue 104] Due to Issue 104, SecAgent only invokes IoDisconnectInterrupt during DriverUnload
    // os->sos_os_intr_unregister();
#endif

    // Finalize SecAgent-App communication
    app_notif_mgr_fini();
    //// Fini asynchronous event notifications
    os->os_app_notif_subsystem_fini();

    __x86_os_cpu_lapic_mapping_fini();

    // Finalize required OS functions
    os_fini();
}

OSSTATUS do_CMD_SECAGENT_PRESENCE_REQ(struct tz_cmd_packet *cmd_packet)
{
    UNUSED(cmd_packet);

    if (!is_sa_running)
        return E_SA_NOT_RUNNING;

    return OS_SUCCESS;
}