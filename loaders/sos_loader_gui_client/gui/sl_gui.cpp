///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "sl_gui.h"

///////////////////////////////////////////////////////////////////////////

Frm_Main::Frm_Main( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	m_menubar1 = new wxMenuBar( 0 );
	m_file = new wxMenu();
	wxMenuItem* m_control_monitor;
	m_control_monitor = new wxMenuItem( m_file, wxID_ANY, wxString( wxT("GreenBox Control Monitor") ) + wxT('\t') + wxT("ALT+C"), wxEmptyString, wxITEM_NORMAL );
	m_file->Append( m_control_monitor );

	wxMenuItem* m_sp_mgr;
	m_sp_mgr = new wxMenuItem( m_file, wxID_ANY, wxString( wxT("SecApp Task Manager") ) + wxT('\t') + wxT("ALT+M"), wxEmptyString, wxITEM_NORMAL );
	m_file->Append( m_sp_mgr );

	m_file->AppendSeparator();

	wxMenuItem* m_about;
	m_about = new wxMenuItem( m_file, wxID_ANY, wxString( wxT("About") ) + wxT('\t') + wxT("ALT+A"), wxEmptyString, wxITEM_NORMAL );
	m_file->Append( m_about );

	m_menubar1->Append( m_file, wxT("Tasks") );

	m_menu2 = new wxMenu();
	wxMenuItem* m_log_show;
	m_log_show = new wxMenuItem( m_menu2, wxID_ANY, wxString( wxT("Show") ) + wxT('\t') + wxT("ALT+S"), wxEmptyString, wxITEM_NORMAL );
	m_menu2->Append( m_log_show );

	m_menubar1->Append( m_menu2, wxT("Log") );

	this->SetMenuBar( m_menubar1 );

	m_statusBar2 = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->SetMinSize( wxSize( 800,600 ) );
	m_bitmap2 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("gui_res/Greenbox.png"), wxBITMAP_TYPE_ANY ), wxPoint( -1,-1 ), wxSize( 800,600 ), 0 );
	m_bitmap2->SetMinSize( wxSize( 800,600 ) );

	fgSizer2->Add( m_bitmap2, 1, wxALL|wxEXPAND, 5 );

	btn_config_sos = new wxButton( this, wxID_ANY, wxT("Configure"), wxDefaultPosition, wxDefaultSize, 0 );
	btn_config_sos->SetFont( wxFont( 16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );

	fgSizer2->Add( btn_config_sos, 0, wxALL|wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer21;
	fgSizer21 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer21->AddGrowableCol( 0 );
	fgSizer21->AddGrowableCol( 1 );
	fgSizer21->AddGrowableRow( 0 );
	fgSizer21->SetFlexibleDirection( wxBOTH );
	fgSizer21->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	btn_run_sos = new wxButton( this, wxID_ANY, wxT("Run"), wxDefaultPosition, wxSize( 400,40 ), 0 );
	btn_run_sos->SetFont( wxFont( 16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );
	btn_run_sos->SetForegroundColour( wxColour( 0, 174, 0 ) );
	btn_run_sos->SetMinSize( wxSize( 400,-1 ) );

	fgSizer21->Add( btn_run_sos, 1, wxALL|wxEXPAND, 5 );

	btn_stop_sos = new wxButton( this, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxSize( 400,40 ), 0 );
	btn_stop_sos->SetFont( wxFont( 16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );
	btn_stop_sos->SetForegroundColour( wxColour( 202, 0, 0 ) );
	btn_stop_sos->Enable( false );
	btn_stop_sos->SetMinSize( wxSize( 400,-1 ) );

	fgSizer21->Add( btn_stop_sos, 0, wxALL|wxEXPAND, 5 );


	fgSizer2->Add( fgSizer21, 1, wxALL|wxEXPAND, 5 );

	g_sos_boot = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,-1 ), wxGA_HORIZONTAL );
	g_sos_boot->SetValue( 0 );
	g_sos_boot->SetMinSize( wxSize( 800,10 ) );

	fgSizer2->Add( g_sos_boot, 0, wxALL, 5 );


	this->SetSizer( fgSizer2 );
	this->Layout();
	m_dtm_probe_timer.SetOwner( this, m_dtm_probe_timer.GetId() );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Frm_Main::Frm_MainOnClose ) );
	this->Connect( wxEVT_SHOW, wxShowEventHandler( Frm_Main::Frm_MainOnShow ) );
	m_file->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( Frm_Main::m_control_monitorOnMenuSelection ), this, m_control_monitor->GetId());
	m_file->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( Frm_Main::m_sp_mgrOnMenuSelection ), this, m_sp_mgr->GetId());
	m_file->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( Frm_Main::m_aboutOnMenuSelection ), this, m_about->GetId());
	m_menu2->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( Frm_Main::m_log_showOnMenuSelection ), this, m_log_show->GetId());
	btn_config_sos->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_Main::btn_config_sosOnButtonClick ), NULL, this );
	btn_config_sos->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( Frm_Main::btn_config_sosOnEnterWindow ), NULL, this );
	btn_config_sos->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( Frm_Main::btn_config_sosOnLeaveWindow ), NULL, this );
	btn_run_sos->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_Main::btn_run_sosOnButtonClick ), NULL, this );
	btn_run_sos->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( Frm_Main::btn_run_sosOnEnterWindow ), NULL, this );
	btn_run_sos->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( Frm_Main::btn_run_sosOnLeaveWindow ), NULL, this );
	btn_stop_sos->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_Main::btn_stop_sosOnButtonClick ), NULL, this );
	btn_stop_sos->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( Frm_Main::btn_stop_sosOnEnterWindow ), NULL, this );
	btn_stop_sos->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( Frm_Main::btn_stop_sosOnLeaveWindow ), NULL, this );
	this->Connect( m_dtm_probe_timer.GetId(), wxEVT_TIMER, wxTimerEventHandler( Frm_Main::m_dtm_probe_timerOnTimer ) );
}

Frm_Main::~Frm_Main()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Frm_Main::Frm_MainOnClose ) );
	this->Disconnect( wxEVT_SHOW, wxShowEventHandler( Frm_Main::Frm_MainOnShow ) );
	btn_config_sos->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_Main::btn_config_sosOnButtonClick ), NULL, this );
	btn_config_sos->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( Frm_Main::btn_config_sosOnEnterWindow ), NULL, this );
	btn_config_sos->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( Frm_Main::btn_config_sosOnLeaveWindow ), NULL, this );
	btn_run_sos->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_Main::btn_run_sosOnButtonClick ), NULL, this );
	btn_run_sos->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( Frm_Main::btn_run_sosOnEnterWindow ), NULL, this );
	btn_run_sos->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( Frm_Main::btn_run_sosOnLeaveWindow ), NULL, this );
	btn_stop_sos->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_Main::btn_stop_sosOnButtonClick ), NULL, this );
	btn_stop_sos->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( Frm_Main::btn_stop_sosOnEnterWindow ), NULL, this );
	btn_stop_sos->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( Frm_Main::btn_stop_sosOnLeaveWindow ), NULL, this );
	this->Disconnect( m_dtm_probe_timer.GetId(), wxEVT_TIMER, wxTimerEventHandler( Frm_Main::m_dtm_probe_timerOnTimer ) );

}

Frm_Log::Frm_Log( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	t_log = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0|wxVSCROLL );
	t_log->Wrap( -1 );
	bSizer1->Add( t_log, 0, wxALL, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );
}

Frm_Log::~Frm_Log()
{
}

Frm_SPMgr::Frm_SPMgr( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 600,600 ), wxDefaultSize );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->AddGrowableRow( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer3->SetMinSize( wxSize( 600,600 ) );
	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Running SecProcesses:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	m_staticText3->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	fgSizer3->Add( m_staticText3, 0, wxALL, 5 );

	m_list_sp = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_list_sp->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	fgSizer3->Add( m_list_sp, 0, wxEXPAND|wxALL, 5 );

	m_soft_kill_sp = new wxButton( this, wxID_ANY, wxT("End SecProcess"), wxDefaultPosition, wxDefaultSize, 0 );
	m_soft_kill_sp->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	m_soft_kill_sp->SetMinSize( wxSize( 200,-1 ) );
	m_soft_kill_sp->SetMaxSize( wxSize( 200,-1 ) );

	fgSizer3->Add( m_soft_kill_sp, 0, wxALL, 5 );


	this->SetSizer( fgSizer3 );
	this->Layout();
	m_sp_list_refresh_timer.SetOwner( this, m_sp_list_refresh_timer.GetId() );
	m_sp_list_refresh_timer.Start( 1000 );


	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Frm_SPMgr::Frm_SPMgrOnClose ) );
	m_list_sp->Connect( wxEVT_COMMAND_LIST_COL_DRAGGING, wxListEventHandler( Frm_SPMgr::m_list_spOnListColDragging ), NULL, this );
	m_list_sp->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( Frm_SPMgr::m_list_spOnListItemSelected ), NULL, this );
	m_soft_kill_sp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_SPMgr::m_soft_kill_spOnButtonClick ), NULL, this );
	this->Connect( m_sp_list_refresh_timer.GetId(), wxEVT_TIMER, wxTimerEventHandler( Frm_SPMgr::m_sp_list_refresh_timerOnTimer ) );
}

Frm_SPMgr::~Frm_SPMgr()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Frm_SPMgr::Frm_SPMgrOnClose ) );
	m_list_sp->Disconnect( wxEVT_COMMAND_LIST_COL_DRAGGING, wxListEventHandler( Frm_SPMgr::m_list_spOnListColDragging ), NULL, this );
	m_list_sp->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( Frm_SPMgr::m_list_spOnListItemSelected ), NULL, this );
	m_soft_kill_sp->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_SPMgr::m_soft_kill_spOnButtonClick ), NULL, this );
	this->Disconnect( m_sp_list_refresh_timer.GetId(), wxEVT_TIMER, wxTimerEventHandler( Frm_SPMgr::m_sp_list_refresh_timerOnTimer ) );

}

Frm_DevConfig::Frm_DevConfig( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer5->SetMinSize( wxSize( 800,600 ) );
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer6->SetMinSize( wxSize( 800,520 ) );
	m_dev_config_type_list = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_dev_config_type_list->SetMinSize( wxSize( 300,520 ) );

	fgSizer6->Add( m_dev_config_type_list, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	t_hid_kbd_selector = new wxStaticText( this, wxID_ANY, wxT("Select Keyboard:"), wxDefaultPosition, wxDefaultSize, 0 );
	t_hid_kbd_selector->Wrap( -1 );
	t_hid_kbd_selector->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	fgSizer7->Add( t_hid_kbd_selector, 0, wxALL, 5 );

	wxArrayString m_hid_kbd_selectorChoices;
	m_hid_kbd_selector = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_hid_kbd_selectorChoices, 0 );
	m_hid_kbd_selector->SetSelection( 0 );
	m_hid_kbd_selector->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	m_hid_kbd_selector->SetMinSize( wxSize( 300,-1 ) );

	fgSizer7->Add( m_hid_kbd_selector, 0, wxALL, 5 );


	bSizer4->Add( fgSizer7, 0, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	t_hid_mouse_selector = new wxStaticText( this, wxID_ANY, wxT("Select Mouse:     "), wxDefaultPosition, wxDefaultSize, 0 );
	t_hid_mouse_selector->Wrap( -1 );
	t_hid_mouse_selector->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	fgSizer9->Add( t_hid_mouse_selector, 0, wxALL, 5 );

	wxArrayString m_hid_mouse_selectorChoices;
	m_hid_mouse_selector = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_hid_mouse_selectorChoices, 0 );
	m_hid_mouse_selector->SetSelection( 0 );
	m_hid_mouse_selector->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	m_hid_mouse_selector->SetMinSize( wxSize( 300,-1 ) );

	fgSizer9->Add( m_hid_mouse_selector, 0, wxALL, 5 );


	bSizer4->Add( fgSizer9, 0, wxEXPAND, 5 );

	m_staticText8 = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	m_staticText8->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	bSizer4->Add( m_staticText8, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	t_disp_resolution_selector = new wxStaticText( this, wxID_ANY, wxT("Window Size:      "), wxDefaultPosition, wxDefaultSize, 0 );
	t_disp_resolution_selector->Wrap( -1 );
	t_disp_resolution_selector->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	fgSizer8->Add( t_disp_resolution_selector, 0, wxALL, 5 );

	wxArrayString m_disp_resolution_selectorChoices;
	m_disp_resolution_selector = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_disp_resolution_selectorChoices, 0 );
	m_disp_resolution_selector->SetSelection( 0 );
	m_disp_resolution_selector->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	m_disp_resolution_selector->SetMinSize( wxSize( 300,-1 ) );

	fgSizer8->Add( m_disp_resolution_selector, 0, wxALL, 5 );


	bSizer4->Add( fgSizer8, 1, wxEXPAND, 5 );

	m_staticText9 = new wxStaticText( this, wxID_ANY, wxT("Key Bindings:\nRelease keyboard/mouse: left CTRL + Right CTRL\nForce terminate GreenBox: left CTRL + Right CTRL + Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	m_staticText9->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );

	bSizer4->Add( m_staticText9, 0, wxALL, 5 );


	fgSizer6->Add( bSizer4, 1, wxALL|wxEXPAND, 5 );


	fgSizer5->Add( fgSizer6, 0, wxEXPAND, 5 );

	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 1, 5, 0, 0 );

	btn_reset_users_profiles = new wxButton( this, wxID_ANY, wxT("Reset Users Profiles"), wxDefaultPosition, wxDefaultSize, 0 );
	btn_reset_users_profiles->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	btn_reset_users_profiles->SetMinSize( wxSize( 200,-1 ) );

	gSizer1->Add( btn_reset_users_profiles, 0, wxALL, 5 );

	t_staticText6 = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	t_staticText6->Wrap( -1 );
	gSizer1->Add( t_staticText6, 0, wxALL, 5 );

	t_staticText5 = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	t_staticText5->Wrap( -1 );
	gSizer1->Add( t_staticText5, 0, wxALL, 5 );

	btn_config_ok = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	btn_config_ok->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	btn_config_ok->SetMinSize( wxSize( 100,-1 ) );

	gSizer1->Add( btn_config_ok, 0, wxALL, 5 );

	btn_config_cancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	btn_config_cancel->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	btn_config_cancel->SetMinSize( wxSize( 100,-1 ) );

	gSizer1->Add( btn_config_cancel, 0, wxALL, 5 );


	fgSizer5->Add( gSizer1, 1, wxEXPAND, 5 );


	this->SetSizer( fgSizer5 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	btn_reset_users_profiles->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_DevConfig::btn_reset_users_profilesOnButtonClick ), NULL, this );
	btn_config_ok->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_DevConfig::btn_config_okOnButtonClick ), NULL, this );
	btn_config_cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_DevConfig::btn_config_cancelOnButtonClick ), NULL, this );
}

Frm_DevConfig::~Frm_DevConfig()
{
	// Disconnect Events
	btn_reset_users_profiles->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_DevConfig::btn_reset_users_profilesOnButtonClick ), NULL, this );
	btn_config_ok->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_DevConfig::btn_config_okOnButtonClick ), NULL, this );
	btn_config_cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Frm_DevConfig::btn_config_cancelOnButtonClick ), NULL, this );

}
