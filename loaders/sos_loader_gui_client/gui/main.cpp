// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
 
#include "sl_gui.h"
#include "SOSLoader_GUIFrm_Main.h"

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);




bool MyApp::OnInit()
{
    SOSLoader_GUIFrm_Main *frame = NULL;

    wxInitAllImageHandlers();

    frame = new SOSLoader_GUIFrm_Main(nullptr);
    frame->Show(true);
    return true;
}




// extern "C"
// {
//     #include <loader_sa_intfs.h>
// }

// static int _query_shared_params_dtm_state(uint64_t* out_dtm_state)
// {
//     OSSTATUS sa_status = OS_SUCCESS;
    
//     // Check: <out_dtm_state> must be valid
//     if(!out_dtm_state)
//         return 1;

//     int a = 1;
//     while (a) {};

//     sa_status = secagent_nsdom_share_data_to_any_ns_process_get(NS_DTM_SECPROCESS_IS_EXIT, out_dtm_state);
//     if(SA_FAIL(sa_status))
//     {
//         // APP_ERR("Failed to get the parameter <NS_DTM_SECPROCESS_IS_EXIT>! SecAgent status:%d\n", sa_status);
//         wxString err;
//         err.Printf("[Superymk] <_query_shared_params_dtm_state> Fail! status:%llu\n", sa_status);
//         wxMessageBox(err);
//         return 1;
//     }

//     // On success
//     return 0;
// } 

// bool MyApp::OnInit()
// {
//     uint64_t dtm_state;
//     (void)_query_shared_params_dtm_state(&dtm_state);

//     return true;
// }