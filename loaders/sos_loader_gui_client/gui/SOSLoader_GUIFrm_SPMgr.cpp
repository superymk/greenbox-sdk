#include "SOSLoader_GUIFrm_SPMgr.h"

SOSLoader_GUIFrm_SPMgr::SOSLoader_GUIFrm_SPMgr(wxWindow *parent)
    : Frm_SPMgr(parent)
{
    // Init window components
    m_sp_list_clear_and_init_columns(this->COL_ID_WIDTH_DEFAULT, this->COL_NAME_WIDTH_DEFAULT);
}

void SOSLoader_GUIFrm_SPMgr::Frm_SPMgrOnClose(wxCloseEvent &event)
{
    sl_frm_spmgr_status_set(false);
    this->Destroy();
}

void SOSLoader_GUIFrm_SPMgr::m_list_spOnListColDragging(wxListEvent &event)
{
    // TODO: Implement m_list_spOnListColDragging
}

void SOSLoader_GUIFrm_SPMgr::m_list_spOnListItemSelected(wxListEvent &event)
{
    // Do nothing
}

void SOSLoader_GUIFrm_SPMgr::m_soft_kill_spOnButtonClick(wxCommandEvent &event)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    long sp_list_selection_idx = this->m_sp_list_get_selected_item_index();

    if (sp_list_selection_idx != -1)
    {
        // One entry in <m_sp_list> is selected
        wxListItem item;
        long pid = 0;

        // Get the ID of the selected SecProcess
        item.m_itemId = sp_list_selection_idx; // sets row
        item.m_col = 0;                        // sets column
        item.m_mask = wxLIST_MASK_TEXT;        // parses item text
        this->m_list_sp->GetItem(item);

        item.m_text.ToLong(&pid);

        // Terminate the SecProcess
        status = client.call_sl_secprocesses_soft_terminate((spid_t)pid);
        // status = sl_secprocesses_soft_terminate((spid_t)pid);
        if (SL_FAIL(status))
        {
            wxString err;
            err.Printf("Soft-Terminate SecProcess (%u) error! status:%lu", pid, status);
            wxMessageBox(err);
        }
    }
}

void SOSLoader_GUIFrm_SPMgr::m_sp_list_refresh_timerOnTimer(wxTimerEvent &event)
{
    SOSLoader_GUI_RPC &client = SOSLoader_GUI_RPC::getInstance();
    SL_STATUS status = SL_SUCCESS;
    struct sp_info_id_name_resp *resp_buf = NULL;
    long sp_list_selection_idx = this->m_sp_list_get_selected_item_index();

    resp_buf = (struct sp_info_id_name_resp *)malloc(sizeof(struct sp_info_id_name_resp));
    if (!resp_buf)
        return;

    status = client.call_sl_secprocesses_get_ids_and_names(resp_buf);
    // status = sl_secprocesses_get_ids_and_names(resp_buf);
    if (SL_FAIL(status))
        goto out;

    // Refresh the result in <m_list_sp>
    {
        // Update list
        int old_ID_width = this->m_list_sp->GetColumnWidth(0);
        int old_Name_width = this->m_list_sp->GetColumnWidth(1);
        unsigned int i = 0;

        m_sp_list_clear_and_init_columns(old_ID_width, old_Name_width);

        for (i = 0; i < resp_buf->num_info && i < SP_INFO_GET_IDS_NAMES_MAX_RESULTS; i++)
        {
            long index;
            wxString id_str = wxString::Format(wxT("%llu"), (uint64_t)resp_buf->info[i].sp_id);
            wxString name_str = wxString::Format(wxT("%s"), resp_buf->info[i].sp_name);

            index = this->m_list_sp->InsertItem(0, id_str);
            this->m_list_sp->SetItem(index, 1, name_str);
        }
    }
    {
        if (sp_list_selection_idx != -1)
        {
            // Restore selection
            if ((sp_list_selection_idx >= 0) && (sp_list_selection_idx < (long)this->m_list_sp->GetItemCount()))
            {
                this->m_list_sp->SetItemState(sp_list_selection_idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
            else
            {
                // The list entry is invalid
                if (this->m_list_sp->GetItemCount() > 0)
                    this->m_list_sp->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
    }

out:
    if (resp_buf)
        free(resp_buf);
}

SOSLoader_GUIFrm_SPMgr::~SOSLoader_GUIFrm_SPMgr(void)
{
}

long SOSLoader_GUIFrm_SPMgr::m_sp_list_get_selected_item_index(void)
{
    return this->m_list_sp->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

/********* Private Functions *********/
void SOSLoader_GUIFrm_SPMgr::m_sp_list_clear_and_init_columns(int col_ID_width, int col_Name_width)
{
    // Clear <m_list_sp>
    this->m_list_sp->ClearAll();

    // Init columns
    this->m_list_sp->InsertColumn(0, _("ID"), wxLIST_FORMAT_LEFT, col_ID_width);
    this->m_list_sp->InsertColumn(1, _("Name"), wxLIST_FORMAT_LEFT, col_Name_width);
}