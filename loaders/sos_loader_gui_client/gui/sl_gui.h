///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/choice.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class Frm_Main
///////////////////////////////////////////////////////////////////////////////
class Frm_Main : public wxFrame
{
	private:

	protected:
		wxMenuBar* m_menubar1;
		wxMenu* m_file;
		wxMenu* m_menu2;
		wxStatusBar* m_statusBar2;
		wxStaticBitmap* m_bitmap2;
		wxButton* btn_config_sos;
		wxButton* btn_run_sos;
		wxButton* btn_stop_sos;
		wxGauge* g_sos_boot;
		wxTimer m_dtm_probe_timer;

		// Virtual event handlers, override them in your derived class
		virtual void Frm_MainOnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void Frm_MainOnShow( wxShowEvent& event ) { event.Skip(); }
		virtual void m_control_monitorOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_sp_mgrOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_aboutOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_log_showOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void btn_config_sosOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void btn_config_sosOnEnterWindow( wxMouseEvent& event ) { event.Skip(); }
		virtual void btn_config_sosOnLeaveWindow( wxMouseEvent& event ) { event.Skip(); }
		virtual void btn_run_sosOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void btn_run_sosOnEnterWindow( wxMouseEvent& event ) { event.Skip(); }
		virtual void btn_run_sosOnLeaveWindow( wxMouseEvent& event ) { event.Skip(); }
		virtual void btn_stop_sosOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void btn_stop_sosOnEnterWindow( wxMouseEvent& event ) { event.Skip(); }
		virtual void btn_stop_sosOnLeaveWindow( wxMouseEvent& event ) { event.Skip(); }
		virtual void m_dtm_probe_timerOnTimer( wxTimerEvent& event ) { event.Skip(); }


	public:

		Frm_Main( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GreenBox"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,800 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Frm_Main();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Frm_Log
///////////////////////////////////////////////////////////////////////////////
class Frm_Log : public wxFrame
{
	private:

	protected:
		wxStaticText* t_log;

	public:

		Frm_Log( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GreenBox Logs"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Frm_Log();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Frm_SPMgr
///////////////////////////////////////////////////////////////////////////////
class Frm_SPMgr : public wxFrame
{
	private:

	protected:
		wxStaticText* m_staticText3;
		wxListCtrl* m_list_sp;
		wxButton* m_soft_kill_sp;
		wxTimer m_sp_list_refresh_timer;

		// Virtual event handlers, override them in your derived class
		virtual void Frm_SPMgrOnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void m_list_spOnListColDragging( wxListEvent& event ) { event.Skip(); }
		virtual void m_list_spOnListItemSelected( wxListEvent& event ) { event.Skip(); }
		virtual void m_soft_kill_spOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_sp_list_refresh_timerOnTimer( wxTimerEvent& event ) { event.Skip(); }


	public:

		Frm_SPMgr( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SecApp Manager"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,600 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Frm_SPMgr();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Frm_DevConfig
///////////////////////////////////////////////////////////////////////////////
class Frm_DevConfig : public wxFrame
{
	private:

	protected:
		wxListCtrl* m_dev_config_type_list;
		wxStaticText* t_hid_kbd_selector;
		wxChoice* m_hid_kbd_selector;
		wxStaticText* t_hid_mouse_selector;
		wxChoice* m_hid_mouse_selector;
		wxStaticText* m_staticText8;
		wxStaticText* t_disp_resolution_selector;
		wxChoice* m_disp_resolution_selector;
		wxStaticText* m_staticText9;
		wxButton* btn_reset_users_profiles;
		wxStaticText* t_staticText6;
		wxStaticText* t_staticText5;
		wxButton* btn_config_ok;
		wxButton* btn_config_cancel;

		// Virtual event handlers, override them in your derived class
		virtual void btn_reset_users_profilesOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void btn_config_okOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void btn_config_cancelOnButtonClick( wxCommandEvent& event ) { event.Skip(); }


	public:

		Frm_DevConfig( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Device Configurations"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,599 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Frm_DevConfig();

};

