
#include "mineraldb.hpp"

class MainFrame: public wxFrame {
    public:
        MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
        void update_gui();
        void draw_mineral_view(int uid);
    private:
        wxMenu *menuFile;
        wxMenu *menuMineral;
        wxMenu *menuHelp;
        wxMenuBar *menuBar;
        wxListBox *mineral_listbox;
        wxTextCtrl *mineral_search;
        wxRadioBox *mineral_orderby;
        wxChoice *mineral_country;
        wxChoice *mineral_state;
        wxChoice *mineral_species;
        wxRichTextCtrl *mineral_view;
        void OnNewMineral(wxCommandEvent& event);
        void OnModifyMineral(wxCommandEvent& event);
        void OnDuplicateMineral(wxCommandEvent& event);
        void OnDeleteMineral(wxCommandEvent& event);
        void OnSelectMineral(wxCommandEvent& event);
        void OnGenReport(wxCommandEvent& event);
        void OnOpen(wxCommandEvent& event);
        void OnSave(wxCommandEvent& event);
        void OnClose(wxCommandEvent& event);
        void import_csv(wxCommandEvent& event);
        void export_csv(wxCommandEvent& event);
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnHelp(wxCommandEvent& event);
        void OnURL(wxTextUrlEvent& event);
        void populate_listbox_evt_country(wxCommandEvent& event);
        void populate_listbox_evt_state(wxCommandEvent& event);
        void populate_listbox_evt_species(wxCommandEvent& event);
        sqlite3 *db;
        void write_table_row(wxString name, std::vector<std::string> data, std::string field);
        void write_table_row_chemf(wxString name, std::vector<std::string> data);
        void write_link_row(std::vector<std::string> data);
        void ReadData(std::string uid);
        int get_minid_from_listbox(bool warn=true);
        std::string db_file_path;
        fs::path get_config_dirname();
        void read_config();
        void write_config();
        void populate_listbox();
        void populate_country_filter();
        void populate_state_filter();
        void populate_species_filter();
        wxDECLARE_EVENT_TABLE();
    };

enum {
    ID_NewMineral = wxID_HIGHEST + 1,
    ID_ModifyMineral,
    ID_DuplicateMineral,
    ID_DeleteMineral,
    ID_SelectMineral,
    ID_SearchMineral,
    ID_OrderByMineral,
    ID_FilterCountry,
    ID_FilterState,
    ID_FilterSpecies,
    ID_ExportCSV,
    ID_ImportCSV,
    ID_HIDEVALUE,
    ID_GenReport,
};

