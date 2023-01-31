
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/richtext/richtextctrl.h>

#include "mineraldb.hpp"
#include "mainframe.h"
#include "addmodframe.h"


wxBEGIN_EVENT_TABLE(AddModFrame, wxFrame)
    EVT_BUTTON(ID_AddMod_Save, AddModFrame::OnSave)
    EVT_BUTTON(ID_AddMod_Cancel, AddModFrame::OnCancel)
wxEND_EVENT_TABLE()


AddModFrame::AddModFrame(MainFrame *parent, const wxString& title, sqlite3 *maindb, int uid)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1000,600), wxDEFAULT_FRAME_STYLE) {

    /* Initialize... */
    db = maindb;
    main_window = parent;
    int border=1;

    /* Main container */
    wxScrolledWindow *panel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, title);
    wxBoxSizer *vsizer = new wxBoxSizer(wxVERTICAL);

    /* Name */
    entry_name = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_name = new wxBoxSizer(wxHORIZONTAL);
    hsizer_name->Add(new wxStaticText(panel, -1, "Name:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_name->Add(entry_name, 4, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_name, 0, wxEXPAND|wxALL, border);

    /* MinID */
    entry_catnum = new wxTextCtrl(panel, -1);
    entry_minid = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_minid = new wxBoxSizer(wxHORIZONTAL);
    hsizer_minid->Add(new wxStaticText(panel, -1, "Catalog N.:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_minid->Add(entry_catnum, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_minid->Add(new wxStaticText(panel, -1, ""), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_minid->Add(new wxStaticText(panel, -1, "minID (mindat):", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_minid->Add(entry_minid, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_minid, 0, wxEXPAND|wxALL, border);

    /* Locality */
    entry_locality = new wxTextCtrl(panel, -1);
    entry_locid = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_locality = new wxBoxSizer(wxHORIZONTAL);
    hsizer_locality->Add(new wxStaticText(panel, -1, "Locality:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_locality->Add(entry_locality, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_locality->Add(new wxStaticText(panel, -1, "Locality ID (mindat):", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_locality->Add(entry_locid, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_locality, 0, wxEXPAND|wxALL, border);

    /* Size */
    wxString sizes[6] = {"", "mm", "cm", "m", "inches", "feet"};
    entry_length = new wxTextCtrl(panel, -1);
    entry_width  = new wxTextCtrl(panel, -1);
    entry_height = new wxTextCtrl(panel, -1);
    entry_size_units = new wxComboBox(panel, -1, "", wxDefaultPosition, wxDefaultSize, 6, sizes, wxCB_READONLY);
    wxBoxSizer *hsizer_size = new wxBoxSizer(wxHORIZONTAL);
    hsizer_size->Add(new wxStaticText(panel, -1, "Size:"), 8, wxALIGN_CENTER|wxALL, border);
    hsizer_size->Add(entry_length, 6, wxALIGN_CENTER|wxALL, border);
    hsizer_size->Add(new wxStaticText(panel, -1, "x", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 3, wxALIGN_CENTER|wxALL, border);
    hsizer_size->Add(entry_width, 6, wxALIGN_CENTER|wxALL, border);
    hsizer_size->Add(new wxStaticText(panel, -1, "x", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 3, wxALIGN_CENTER|wxALL, border);
    hsizer_size->Add(entry_height, 6, wxALIGN_CENTER|wxALL, border);
    hsizer_size->Add(entry_size_units, 8, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_size, 0, wxEXPAND|wxALL, border);

    /* Weight*/
    wxString size_categories[9] = {"", "Micromount (microscope)", "Thumbnail (<1 inch)", "Toenail (<1.25 inch)",
        "Miniature (3-6 cm)", "Small cabinet (5-7 cm)", "Cabinet (6-10 cm)", "Large cabinet (10-15 cm)", "Extra Large (>15 cm)"};
    wxString weight_units[7] = {"", "g", "mg", "kg", "oz", "lb", "carats"};
    entry_weight = new wxTextCtrl(panel, -1);
    entry_weight_units = new wxComboBox(panel, -1, "", wxDefaultPosition, wxDefaultSize, 7, weight_units, wxCB_READONLY);
    entry_size_category = new wxComboBox(panel, -1, "", wxDefaultPosition, wxDefaultSize, 9, size_categories, wxCB_READONLY);
    wxBoxSizer *hsizer_weight = new wxBoxSizer(wxHORIZONTAL);
    hsizer_weight->Add(new wxStaticText(panel, -1, "Weight:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_weight->Add(entry_weight, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_weight->Add(entry_weight_units, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_weight->Add(entry_size_category, 2, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_weight, 0, wxEXPAND|wxALL, border);

    /* Acquisition */
    wxString days[32] = {"", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14",
        "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};
    wxString months[13] = {"", "January", "February", "March", "April",
        "May", "June", "July", "August", "September", "October", "November", "December"};
    entry_acquisition_day = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 32, days, wxCB_READONLY);
    entry_acquisition_month = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 13, months, wxCB_READONLY);
    time_t now = time(0);
    tm *ltm = localtime(&now);
    entry_acquisition_year = new wxTextCtrl(panel, -1, std::to_string(1900+ltm->tm_year));
    entry_acquisition_source = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_acquisition = new wxBoxSizer(wxHORIZONTAL);
    hsizer_acquisition->Add(new wxStaticText(panel, -1, "Acquisition:"), 3, wxALIGN_CENTER|wxALL, border);
    hsizer_acquisition->Add(entry_acquisition_day, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_acquisition->Add(entry_acquisition_month, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_acquisition->Add(entry_acquisition_year, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_acquisition->Add(new wxStaticText(panel, -1, "From:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 2, wxALIGN_CENTER|wxALL, border);
    hsizer_acquisition->Add(entry_acquisition_source, 4, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_acquisition, 0, wxEXPAND|wxALL, border);

    /* Deaccessioned */
    entry_deaccessioned_day = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 32, days, wxCB_READONLY);
    entry_deaccessioned_month = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 13, months, wxCB_READONLY);
    entry_deaccessioned_year = new wxTextCtrl(panel, -1);
    entry_deaccessioned_to = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_deaccessioned = new wxBoxSizer(wxHORIZONTAL);
    hsizer_deaccessioned->Add(new wxStaticText(panel, -1, "Deaccessioned:"), 3, wxALIGN_CENTER|wxALL, border);
    hsizer_deaccessioned->Add(entry_deaccessioned_day, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_deaccessioned->Add(entry_deaccessioned_month, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_deaccessioned->Add(entry_deaccessioned_year, 2, wxALIGN_CENTER|wxALL, border);
    hsizer_deaccessioned->Add(new wxStaticText(panel, -1, "To:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 2, wxALIGN_CENTER|wxALL, border);
    hsizer_deaccessioned->Add(entry_deaccessioned_to, 4, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_deaccessioned, 0, wxEXPAND|wxALL, border);

    /* Collection */
    entry_collection = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_collection = new wxBoxSizer(wxHORIZONTAL);
    hsizer_collection->Add(new wxStaticText(panel, -1, "Collection:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_collection->Add(entry_collection, 4, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_collection, 0, wxEXPAND|wxALL, border);

    /* Value */
    entry_value = new wxTextCtrl(panel, -1);
    entry_price = new wxTextCtrl(panel, -1);
    entry_selfcollected = new wxCheckBox(panel, -1, "Self collected:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    wxBoxSizer *hsizer_value = new wxBoxSizer(wxHORIZONTAL);
    hsizer_value->Add(new wxStaticText(panel, -1, "Estimated value:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_value->Add(entry_value, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_value->Add(new wxStaticText(panel, -1, "Price paid:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_value->Add(entry_price, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_value->Add(entry_selfcollected, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_value, 0, wxEXPAND|wxALL, border);

    /* Species title row */
    wxBoxSizer *hsizer_species_l = new wxBoxSizer(wxHORIZONTAL);
    hsizer_species_l->Add(new wxStaticText(panel, -1, ""), 1, wxALL, border);
    hsizer_species_l->Add(new wxStaticText(panel, -1, "Species 1"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_species_l->Add(new wxStaticText(panel, -1, "Species 2"), 1, wxALL, border);
    hsizer_species_l->Add(new wxStaticText(panel, -1, "Species 3"), 1, wxALL, border);
    hsizer_species_l->Add(new wxStaticText(panel, -1, "Species 4"), 1, wxALL, border);
    vsizer->Add(hsizer_species_l, 0, wxEXPAND|wxALL, 5);

    /* Species */
    entry_s1_species = new wxTextCtrl(panel, -1);
    entry_s2_species = new wxTextCtrl(panel, -1);
    entry_s3_species = new wxTextCtrl(panel, -1);
    entry_s4_species = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_species = new wxBoxSizer(wxHORIZONTAL);
    hsizer_species->Add(new wxStaticText(panel, -1, "Species:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_species->Add(entry_s1_species, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_species->Add(entry_s2_species, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_species->Add(entry_s3_species, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_species->Add(entry_s4_species, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_species, 0, wxEXPAND|wxALL, border);

    /* Variety */
    entry_s1_variety = new wxTextCtrl(panel, -1);
    entry_s2_variety = new wxTextCtrl(panel, -1);
    entry_s3_variety = new wxTextCtrl(panel, -1);
    entry_s4_variety = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_variety = new wxBoxSizer(wxHORIZONTAL);
    hsizer_variety->Add(new wxStaticText(panel, -1, "Variety:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_variety->Add(entry_s1_variety, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_variety->Add(entry_s2_variety, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_variety->Add(entry_s3_variety, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_variety->Add(entry_s4_variety, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_variety, 0, wxEXPAND|wxALL, border);
 
    /* Class */
    entry_s1_class = new wxTextCtrl(panel, -1);
    entry_s2_class = new wxTextCtrl(panel, -1);
    entry_s3_class = new wxTextCtrl(panel, -1);
    entry_s4_class = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_class = new wxBoxSizer(wxHORIZONTAL);
    hsizer_class->Add(new wxStaticText(panel, -1, "Class:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_class->Add(entry_s1_class, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_class->Add(entry_s2_class, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_class->Add(entry_s3_class, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_class->Add(entry_s4_class, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_class, 0, wxEXPAND|wxALL, border);
 
    /* Chemical Formula */
    entry_s1_chemf = new wxTextCtrl(panel, -1);
    entry_s2_chemf = new wxTextCtrl(panel, -1);
    entry_s3_chemf = new wxTextCtrl(panel, -1);
    entry_s4_chemf = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_chemf = new wxBoxSizer(wxHORIZONTAL);
    hsizer_chemf->Add(new wxStaticText(panel, -1, "Chemical Formula:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_chemf->Add(entry_s1_chemf, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_chemf->Add(entry_s2_chemf, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_chemf->Add(entry_s3_chemf, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_chemf->Add(entry_s4_chemf, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_chemf, 0, wxEXPAND|wxALL, border);

    /* Color */
    entry_s1_color = new wxTextCtrl(panel, -1);
    entry_s2_color = new wxTextCtrl(panel, -1);
    entry_s3_color = new wxTextCtrl(panel, -1);
    entry_s4_color = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_color = new wxBoxSizer(wxHORIZONTAL);
    hsizer_color->Add(new wxStaticText(panel, -1, "Color:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_color->Add(entry_s1_color, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_color->Add(entry_s2_color, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_color->Add(entry_s3_color, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_color->Add(entry_s4_color, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_color, 0, wxEXPAND|wxALL, border);

    /* Transparency */
    wxString transp[4] = { "", "Opaque", "Translucent", "Transparent" };
    entry_s1_transp = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 4, transp, wxCB_READONLY);
    entry_s2_transp = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 4, transp, wxCB_READONLY);
    entry_s3_transp = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 4, transp, wxCB_READONLY);
    entry_s4_transp = new wxComboBox(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 4, transp, wxCB_READONLY);
    wxBoxSizer *hsizer_transp = new wxBoxSizer(wxHORIZONTAL);
    hsizer_transp->Add(new wxStaticText(panel, -1, "Transparency:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_transp->Add(entry_s1_transp, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_transp->Add(entry_s2_transp, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_transp->Add(entry_s3_transp, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_transp->Add(entry_s4_transp, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_transp, 0, wxEXPAND|wxALL, border);

    /* Habit */
    entry_s1_habit = new wxTextCtrl(panel, -1);
    entry_s2_habit = new wxTextCtrl(panel, -1);
    entry_s3_habit = new wxTextCtrl(panel, -1);
    entry_s4_habit = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_habit = new wxBoxSizer(wxHORIZONTAL);
    hsizer_habit->Add(new wxStaticText(panel, -1, "Habit:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_habit->Add(entry_s1_habit, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_habit->Add(entry_s2_habit, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_habit->Add(entry_s3_habit, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_habit->Add(entry_s4_habit, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_habit, 0, wxEXPAND|wxALL, border);
 
    /* Fluorescence (SW) */
    entry_s1_flsw = new wxTextCtrl(panel, -1);
    entry_s2_flsw = new wxTextCtrl(panel, -1);
    entry_s3_flsw = new wxTextCtrl(panel, -1);
    entry_s4_flsw = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_flsw = new wxBoxSizer(wxHORIZONTAL);
    hsizer_flsw->Add(new wxStaticText(panel, -1, "Fluorescence (SW):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flsw->Add(entry_s1_flsw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flsw->Add(entry_s2_flsw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flsw->Add(entry_s3_flsw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flsw->Add(entry_s4_flsw, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_flsw, 0, wxEXPAND|wxALL, border);
 
    /* Fluorescence (MW) */
    entry_s1_flmw = new wxTextCtrl(panel, -1);
    entry_s2_flmw = new wxTextCtrl(panel, -1);
    entry_s3_flmw = new wxTextCtrl(panel, -1);
    entry_s4_flmw = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_flmw = new wxBoxSizer(wxHORIZONTAL);
    hsizer_flmw->Add(new wxStaticText(panel, -1, "Fluorescence (MW):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flmw->Add(entry_s1_flmw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flmw->Add(entry_s2_flmw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flmw->Add(entry_s3_flmw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_flmw->Add(entry_s4_flmw, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_flmw, 0, wxEXPAND|wxALL, border);
 
    /* Fluorescence (LW) */
    entry_s1_fllw = new wxTextCtrl(panel, -1);
    entry_s2_fllw = new wxTextCtrl(panel, -1);
    entry_s3_fllw = new wxTextCtrl(panel, -1);
    entry_s4_fllw = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_fllw = new wxBoxSizer(wxHORIZONTAL);
    hsizer_fllw->Add(new wxStaticText(panel, -1, "Fluorescence (LW):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fllw->Add(entry_s1_fllw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fllw->Add(entry_s2_fllw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fllw->Add(entry_s3_fllw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fllw->Add(entry_s4_fllw, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_fllw, 0, wxEXPAND|wxALL, border);
 
    /* Fluorescence (405nm) */
    entry_s1_fl405 = new wxTextCtrl(panel, -1);
    entry_s2_fl405 = new wxTextCtrl(panel, -1);
    entry_s3_fl405 = new wxTextCtrl(panel, -1);
    entry_s4_fl405 = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_fl405 = new wxBoxSizer(wxHORIZONTAL);
    hsizer_fl405->Add(new wxStaticText(panel, -1, "Fluorescence (405nm):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fl405->Add(entry_s1_fl405, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fl405->Add(entry_s2_fl405, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fl405->Add(entry_s3_fl405, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_fl405->Add(entry_s4_fl405, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_fl405, 0, wxEXPAND|wxALL, border);
 
    /* Phoshorescence (SW) */
    entry_s1_phsw = new wxTextCtrl(panel, -1);
    entry_s2_phsw = new wxTextCtrl(panel, -1);
    entry_s3_phsw = new wxTextCtrl(panel, -1);
    entry_s4_phsw = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_phsw = new wxBoxSizer(wxHORIZONTAL);
    hsizer_phsw->Add(new wxStaticText(panel, -1, "Phoshorescence (SW):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phsw->Add(entry_s1_phsw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phsw->Add(entry_s2_phsw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phsw->Add(entry_s3_phsw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phsw->Add(entry_s4_phsw, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_phsw, 0, wxEXPAND|wxALL, border);
 
    /* Phoshorescence (MW) */
    entry_s1_phmw = new wxTextCtrl(panel, -1);
    entry_s2_phmw = new wxTextCtrl(panel, -1);
    entry_s3_phmw = new wxTextCtrl(panel, -1);
    entry_s4_phmw = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_phmw = new wxBoxSizer(wxHORIZONTAL);
    hsizer_phmw->Add(new wxStaticText(panel, -1, "Phoshorescence (MW):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phmw->Add(entry_s1_phmw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phmw->Add(entry_s2_phmw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phmw->Add(entry_s3_phmw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phmw->Add(entry_s4_phmw, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_phmw, 0, wxEXPAND|wxALL, border);
 
    /* Phoshorescence (LW) */
    entry_s1_phlw = new wxTextCtrl(panel, -1);
    entry_s2_phlw = new wxTextCtrl(panel, -1);
    entry_s3_phlw = new wxTextCtrl(panel, -1);
    entry_s4_phlw = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_phlw = new wxBoxSizer(wxHORIZONTAL);
    hsizer_phlw->Add(new wxStaticText(panel, -1, "Phoshorescence (LW):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phlw->Add(entry_s1_phlw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phlw->Add(entry_s2_phlw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phlw->Add(entry_s3_phlw, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_phlw->Add(entry_s4_phlw, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_phlw, 0, wxEXPAND|wxALL, border);
 
    /* Phoshorescence (405nm) */
    entry_s1_ph405 = new wxTextCtrl(panel, -1);
    entry_s2_ph405 = new wxTextCtrl(panel, -1);
    entry_s3_ph405 = new wxTextCtrl(panel, -1);
    entry_s4_ph405 = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_ph405 = new wxBoxSizer(wxHORIZONTAL);
    hsizer_ph405->Add(new wxStaticText(panel, -1, "Phoshorescence (405nm):"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_ph405->Add(entry_s1_ph405, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_ph405->Add(entry_s2_ph405, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_ph405->Add(entry_s3_ph405, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_ph405->Add(entry_s4_ph405, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_ph405, 0, wxEXPAND|wxALL, border);
 
    /* Tenebrescence */
    entry_s1_tenebr = new wxTextCtrl(panel, -1);
    entry_s2_tenebr = new wxTextCtrl(panel, -1);
    entry_s3_tenebr = new wxTextCtrl(panel, -1);
    entry_s4_tenebr = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_tenebr = new wxBoxSizer(wxHORIZONTAL);
    hsizer_tenebr->Add(new wxStaticText(panel, -1, "Tenebrescence:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tenebr->Add(entry_s1_tenebr, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tenebr->Add(entry_s2_tenebr, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tenebr->Add(entry_s3_tenebr, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tenebr->Add(entry_s4_tenebr, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_tenebr, 0, wxEXPAND|wxALL, border);

    /* Triboluminescence */
    entry_s1_tribo = new wxTextCtrl(panel, -1);
    entry_s2_tribo = new wxTextCtrl(panel, -1);
    entry_s3_tribo = new wxTextCtrl(panel, -1);
    entry_s4_tribo = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_tribo = new wxBoxSizer(wxHORIZONTAL);
    hsizer_tribo->Add(new wxStaticText(panel, -1, "Triboluminescence:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tribo->Add(entry_s1_tribo, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tribo->Add(entry_s2_tribo, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tribo->Add(entry_s3_tribo, 1, wxALIGN_CENTER|wxALL, border);
    hsizer_tribo->Add(entry_s4_tribo, 1, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_tribo, 0, wxEXPAND|wxALL, border);

    /* Radioactivity */
    entry_radioact = new wxTextCtrl(panel, -1);
    wxBoxSizer *hsizer_radioact = new wxBoxSizer(wxHORIZONTAL);
    hsizer_radioact->Add(new wxStaticText(panel, -1, "Radioactivity:"), 1, wxALIGN_CENTER|wxALL, border);
    hsizer_radioact->Add(entry_radioact, 4, wxALIGN_CENTER|wxALL, border);
    vsizer->Add(hsizer_radioact, 0, wxEXPAND|wxALL, border);

    /* Description */
    entry_description = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    wxBoxSizer *hsizer_description = new wxBoxSizer(wxHORIZONTAL);
    hsizer_description->Add(new wxStaticText(panel, -1, "Description:"), 1, wxEXPAND|wxALL, border);
    hsizer_description->Add(entry_description, 4, wxEXPAND|wxALL, border);
    vsizer->Add(hsizer_description, 1, wxEXPAND|wxALL, border);

    /* Notes */
    entry_notes = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    wxBoxSizer *hsizer_notes = new wxBoxSizer(wxHORIZONTAL);
    hsizer_notes->Add(new wxStaticText(panel, -1, "Notes:"), 1, wxEXPAND|wxALL, border);
    hsizer_notes->Add(entry_notes, 4, wxEXPAND|wxALL, border);
    vsizer->Add(hsizer_notes, 1, wxEXPAND|wxALL, border);

    /* Owners */
    entry_owners = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    wxBoxSizer *hsizer_owners = new wxBoxSizer(wxHORIZONTAL);
    hsizer_owners->Add(new wxStaticText(panel, -1, "Previous owners:"), 1, wxEXPAND|wxALL, border);
    hsizer_owners->Add(entry_owners, 4, wxEXPAND|wxALL, border);
    vsizer->Add(hsizer_owners, 1, wxEXPAND|wxALL, border);

    /* ok cancel buttons */
    wxBoxSizer *hsizer_buttons = new wxBoxSizer(wxHORIZONTAL);
    wxButton *button_save = new wxButton(panel, ID_AddMod_Save, wxString("Save"));
    wxButton *button_cancel = new wxButton(panel, ID_AddMod_Cancel, wxString("Cancel"));
    hsizer_buttons->Add(new wxStaticText(panel, -1, ""), 3, wxALL, border);
    hsizer_buttons->Add(button_cancel, 0, wxALL, border);
    hsizer_buttons->Add(button_save, 0, wxALL, border);
    vsizer->Add(hsizer_buttons, 0, wxEXPAND|wxALL, border);

    /* Fit everything in the panel and add scrollbars */
    panel->SetSizerAndFit(vsizer);
    panel->FitInside();
    panel->SetScrollRate(5, 5);

    /* If given uid, populate from with data from db */
    modifying = uid;
    if (uid>=0) {
        PopulateForm(uid);
    }
}

void AddModFrame::OnSave(wxCommandEvent& event) {

    std::vector<std::string> data(data_header.size());
    std::string temp;
    int ndx;

    ndx = db_get_field_index("ID");         if (ndx>=0) data[ndx] = entry_catnum->GetValue();
    ndx = db_get_field_index("MINID");      if (ndx>=0) data[ndx] = entry_minid->GetValue();
    ndx = db_get_field_index("NAME");       if (ndx>=0) data[ndx] = entry_name->GetValue();
    ndx = db_get_field_index("LOCALITY");   if (ndx>=0) data[ndx] = entry_locality->GetValue();
    ndx = db_get_field_index("LOCALITY_ID");if (ndx>=0) data[ndx] = entry_locid->GetValue();
    ndx = db_get_field_index("LENGTH");     if (ndx>=0) data[ndx] = entry_length->GetValue();
    ndx = db_get_field_index("WIDTH");      if (ndx>=0) data[ndx] = entry_width->GetValue();
    ndx = db_get_field_index("HEIGHT");     if (ndx>=0) data[ndx] = entry_height->GetValue();
    ndx = db_get_field_index("SIZE_UNITS"); if (ndx>=0) data[ndx] = entry_size_units->GetStringSelection();
    ndx = db_get_field_index("SIZE_CATEGORY");if (ndx>=0) data[ndx] = entry_size_category->GetStringSelection();
    ndx = db_get_field_index("WEIGHT");       if (ndx>=0) data[ndx] = entry_weight->GetValue();
    ndx = db_get_field_index("WEIGHT_UNITS"); if (ndx>=0) data[ndx] = entry_weight_units->GetStringSelection();
    ndx = db_get_field_index("ACQUISITION_DAY");   if (ndx>=0) data[ndx] = entry_acquisition_day->GetStringSelection();
    ndx = db_get_field_index("ACQUISITION_MONTH"); if (ndx>=0) data[ndx] = entry_acquisition_month->GetStringSelection();
    ndx = db_get_field_index("ACQUISITION_YEAR");  if (ndx>=0) data[ndx] = entry_acquisition_year->GetValue();
    ndx = db_get_field_index("ACQUISITION_SOURCE");if (ndx>=0) data[ndx] = entry_acquisition_source->GetValue();
    ndx = db_get_field_index("DEACCESSIONED_DAY");   if (ndx>=0) data[ndx] = entry_deaccessioned_day->GetStringSelection();
    ndx = db_get_field_index("DEACCESSIONED_MONTH"); if (ndx>=0) data[ndx] = entry_deaccessioned_month->GetStringSelection();
    ndx = db_get_field_index("DEACCESSIONED_YEAR");  if (ndx>=0) data[ndx] = entry_deaccessioned_year->GetValue();
    ndx = db_get_field_index("DEACCESSIONED_TO");if (ndx>=0) data[ndx] = entry_deaccessioned_to->GetValue();
    std::string d = db_get_field(data, "DEACCESSIONED_DAY") + db_get_field(data, "DEACCESSIONED_MONTH") +
                    db_get_field(data, "DEACCESSIONED_YEAR") + db_get_field(data, "DEACCESSIONED_TO");
    if (d.size()>0) {
        ndx = db_get_field_index("DEACCESSIONED");if (ndx>=0) data[ndx] = "1";
    }

    ndx = db_get_field_index("COLLECTION");    if (ndx>=0) data[ndx] = entry_collection->GetValue();
    ndx = db_get_field_index("VALUE");         if (ndx>=0) data[ndx] = entry_value->GetValue();
    ndx = db_get_field_index("PRICE");         if (ndx>=0) data[ndx] = entry_price->GetValue();
    ndx = db_get_field_index("SELFCOLLECTED"); if (ndx>=0) data[ndx] = entry_selfcollected->GetValue() ? "1" : "0";

    ndx = db_get_field_index("S1_SPECIES"); if (ndx>=0) data[ndx] = entry_s1_species->GetValue();
    ndx = db_get_field_index("S1_VARIETY"); if (ndx>=0) data[ndx] = entry_s1_variety->GetValue();
    ndx = db_get_field_index("S1_CLASS");   if (ndx>=0) data[ndx] = entry_s1_class->GetValue();
    ndx = db_get_field_index("S1_CHEMF");   if (ndx>=0) {    temp = entry_s1_chemf->GetValue().ToStdString(); data[ndx] = strip_unicode(temp);}
    ndx = db_get_field_index("S1_COLOR");   if (ndx>=0) data[ndx] = entry_s1_color->GetValue();
    ndx = db_get_field_index("S1_TRANSP");  if (ndx>=0) data[ndx] = entry_s1_transp->GetValue();
    ndx = db_get_field_index("S1_HABIT");   if (ndx>=0) data[ndx] = entry_s1_habit->GetValue();
    ndx = db_get_field_index("S1_FLSW");    if (ndx>=0) data[ndx] = entry_s1_flsw->GetValue();
    ndx = db_get_field_index("S1_FLMW");    if (ndx>=0) data[ndx] = entry_s1_flmw->GetValue();
    ndx = db_get_field_index("S1_FLLW");    if (ndx>=0) data[ndx] = entry_s1_fllw->GetValue();
    ndx = db_get_field_index("S1_FL405");   if (ndx>=0) data[ndx] = entry_s1_fl405->GetValue();
    ndx = db_get_field_index("S1_PHSW");    if (ndx>=0) data[ndx] = entry_s1_phsw->GetValue();
    ndx = db_get_field_index("S1_PHMW");    if (ndx>=0) data[ndx] = entry_s1_phmw->GetValue();
    ndx = db_get_field_index("S1_PHLW");    if (ndx>=0) data[ndx] = entry_s1_phlw->GetValue();
    ndx = db_get_field_index("S1_PH405");   if (ndx>=0) data[ndx] = entry_s1_ph405->GetValue();
    ndx = db_get_field_index("S1_TENEBR");  if (ndx>=0) data[ndx] = entry_s1_tenebr->GetValue();
    ndx = db_get_field_index("S1_TRIBO");   if (ndx>=0) data[ndx] = entry_s1_tribo->GetValue();

    ndx = db_get_field_index("S2_SPECIES"); if (ndx>=0) data[ndx] = entry_s2_species->GetValue();
    ndx = db_get_field_index("S2_VARIETY"); if (ndx>=0) data[ndx] = entry_s2_variety->GetValue();
    ndx = db_get_field_index("S2_CLASS");   if (ndx>=0) data[ndx] = entry_s2_class->GetValue();
    ndx = db_get_field_index("S2_CHEMF");   if (ndx>=0) {    temp = entry_s2_chemf->GetValue().ToStdString(); data[ndx] = strip_unicode(temp);}
    ndx = db_get_field_index("S2_COLOR");   if (ndx>=0) data[ndx] = entry_s2_color->GetValue();
    ndx = db_get_field_index("S2_TRANSP");  if (ndx>=0) data[ndx] = entry_s2_transp->GetValue();
    ndx = db_get_field_index("S2_HABIT");   if (ndx>=0) data[ndx] = entry_s2_habit->GetValue();
    ndx = db_get_field_index("S2_FLSW");    if (ndx>=0) data[ndx] = entry_s2_flsw->GetValue();
    ndx = db_get_field_index("S2_FLMW");    if (ndx>=0) data[ndx] = entry_s2_flmw->GetValue();
    ndx = db_get_field_index("S2_FLLW");    if (ndx>=0) data[ndx] = entry_s2_fllw->GetValue();
    ndx = db_get_field_index("S2_FL405");   if (ndx>=0) data[ndx] = entry_s2_fl405->GetValue();
    ndx = db_get_field_index("S2_PHSW");    if (ndx>=0) data[ndx] = entry_s2_phsw->GetValue();
    ndx = db_get_field_index("S2_PHMW");    if (ndx>=0) data[ndx] = entry_s2_phmw->GetValue();
    ndx = db_get_field_index("S2_PHLW");    if (ndx>=0) data[ndx] = entry_s2_phlw->GetValue();
    ndx = db_get_field_index("S2_PH405");   if (ndx>=0) data[ndx] = entry_s2_ph405->GetValue();
    ndx = db_get_field_index("S2_TENEBR");  if (ndx>=0) data[ndx] = entry_s2_tenebr->GetValue();
    ndx = db_get_field_index("S2_TRIBO");   if (ndx>=0) data[ndx] = entry_s2_tribo->GetValue();

    ndx = db_get_field_index("S3_SPECIES"); if (ndx>=0) data[ndx] = entry_s3_species->GetValue();
    ndx = db_get_field_index("S3_VARIETY"); if (ndx>=0) data[ndx] = entry_s3_variety->GetValue();
    ndx = db_get_field_index("S3_CLASS");   if (ndx>=0) data[ndx] = entry_s3_class->GetValue();
    ndx = db_get_field_index("S3_CHEMF");   if (ndx>=0) {    temp = entry_s3_chemf->GetValue().ToStdString(); data[ndx] = strip_unicode(temp);}
    ndx = db_get_field_index("S3_COLOR");   if (ndx>=0) data[ndx] = entry_s3_color->GetValue();
    ndx = db_get_field_index("S3_TRANSP");  if (ndx>=0) data[ndx] = entry_s3_transp->GetValue();
    ndx = db_get_field_index("S3_HABIT");   if (ndx>=0) data[ndx] = entry_s3_habit->GetValue();
    ndx = db_get_field_index("S3_FLSW");    if (ndx>=0) data[ndx] = entry_s3_flsw->GetValue();
    ndx = db_get_field_index("S3_FLMW");    if (ndx>=0) data[ndx] = entry_s3_flmw->GetValue();
    ndx = db_get_field_index("S3_FLLW");    if (ndx>=0) data[ndx] = entry_s3_fllw->GetValue();
    ndx = db_get_field_index("S3_FL405");   if (ndx>=0) data[ndx] = entry_s3_fl405->GetValue();
    ndx = db_get_field_index("S3_PHSW");    if (ndx>=0) data[ndx] = entry_s3_phsw->GetValue();
    ndx = db_get_field_index("S3_PHMW");    if (ndx>=0) data[ndx] = entry_s3_phmw->GetValue();
    ndx = db_get_field_index("S3_PHLW");    if (ndx>=0) data[ndx] = entry_s3_phlw->GetValue();
    ndx = db_get_field_index("S3_PH405");   if (ndx>=0) data[ndx] = entry_s3_ph405->GetValue();
    ndx = db_get_field_index("S3_TENEBR");  if (ndx>=0) data[ndx] = entry_s3_tenebr->GetValue();
    ndx = db_get_field_index("S3_TRIBO");   if (ndx>=0) data[ndx] = entry_s3_tribo->GetValue();

    ndx = db_get_field_index("S4_SPECIES"); if (ndx>=0) data[ndx] = entry_s4_species->GetValue();
    ndx = db_get_field_index("S4_VARIETY"); if (ndx>=0) data[ndx] = entry_s4_variety->GetValue();
    ndx = db_get_field_index("S4_CLASS");   if (ndx>=0) data[ndx] = entry_s4_class->GetValue();
    ndx = db_get_field_index("S4_CHEMF");   if (ndx>=0) {    temp = entry_s4_chemf->GetValue().ToStdString(); data[ndx] = strip_unicode(temp);}
    ndx = db_get_field_index("S4_COLOR");   if (ndx>=0) data[ndx] = entry_s4_color->GetValue();
    ndx = db_get_field_index("S4_TRANSP");  if (ndx>=0) data[ndx] = entry_s4_transp->GetValue();
    ndx = db_get_field_index("S4_HABIT");   if (ndx>=0) data[ndx] = entry_s4_habit->GetValue();
    ndx = db_get_field_index("S4_FLSW");    if (ndx>=0) data[ndx] = entry_s4_flsw->GetValue();
    ndx = db_get_field_index("S4_FLMW");    if (ndx>=0) data[ndx] = entry_s4_flmw->GetValue();
    ndx = db_get_field_index("S4_FLLW");    if (ndx>=0) data[ndx] = entry_s4_fllw->GetValue();
    ndx = db_get_field_index("S4_FL405");   if (ndx>=0) data[ndx] = entry_s4_fl405->GetValue();
    ndx = db_get_field_index("S4_PHSW");    if (ndx>=0) data[ndx] = entry_s4_phsw->GetValue();
    ndx = db_get_field_index("S4_PHMW");    if (ndx>=0) data[ndx] = entry_s4_phmw->GetValue();
    ndx = db_get_field_index("S4_PHLW");    if (ndx>=0) data[ndx] = entry_s4_phlw->GetValue();
    ndx = db_get_field_index("S4_PH405");   if (ndx>=0) data[ndx] = entry_s4_ph405->GetValue();
    ndx = db_get_field_index("S4_TENEBR");  if (ndx>=0) data[ndx] = entry_s4_tenebr->GetValue();
    ndx = db_get_field_index("S4_TRIBO");   if (ndx>=0) data[ndx] = entry_s4_tribo->GetValue();

    ndx = db_get_field_index("RADIOACTIVITY"); if (ndx>=0) data[ndx] = entry_radioact->GetValue();
    ndx = db_get_field_index("DESCRIPTION");   if (ndx>=0) data[ndx] = entry_description->GetValue();
    ndx = db_get_field_index("NOTES");         if (ndx>=0) data[ndx] = entry_notes->GetValue();
    ndx = db_get_field_index("OWNERS");        if (ndx>=0) data[ndx] = entry_owners->GetValue();

    std::string errmsg = "";
    int success_id = db_addmod_mineral(db, data, modifying, &errmsg);
    if (success_id<-1) {
        wxLogMessage(wxString(errmsg));
    }

    main_window->update_gui();
    main_window->draw_mineral_view(success_id);
    Close(true);
    return;
}

void AddModFrame::OnCancel(wxCommandEvent& event) {
    Close(true);
}

void AddModFrame::PopulateForm(int uid) {

    std::string errmsg = "";
    std::vector<std::string> data = db_get_data(db, uid, &errmsg);
    if (errmsg.size()>0) {
        return;
    }

    entry_catnum->AppendText         (db_get_field(data, "ID", false));
    entry_minid->AppendText      (db_get_field(data, "MINID", false));
    entry_name->AppendText       (db_get_field(data, "NAME", false));
    entry_locality->AppendText   (db_get_field(data, "LOCALITY", false));
    entry_locid->AppendText      (db_get_field(data, "LOCALITY_ID", false));
    entry_length->SetValue       (db_get_field(data, "LENGTH", false));
    entry_width->SetValue        (db_get_field(data, "WIDTH", false));
    entry_height->SetValue       (db_get_field(data, "HEIGHT", false));
    entry_size_units->SetValue   (db_get_field(data, "SIZE_UNITS", false));
    entry_size_category->SetValue(db_get_field(data, "SIZE_CATEGORY", false));
    entry_weight->AppendText     (db_get_field(data, "WEIGHT", false));
    entry_weight_units->SetValue (db_get_field(data, "WEIGHT_UNITS", false));
    entry_acquisition_day->SetValue(db_get_field(data, "ACQUISITION_DAY", false));
    entry_acquisition_month->SetValue(db_get_field(data, "ACQUISITION_MONTH", false));
    entry_acquisition_year->SetValue(db_get_field(data, "ACQUISITION_YEAR", false));
    entry_acquisition_source->AppendText(db_get_field(data, "ACQUISITION_SOURCE", false));
    entry_deaccessioned_day->SetValue(db_get_field(data, "DEACCESSIONED_DAY", false));
    entry_deaccessioned_month->SetValue(db_get_field(data, "DEACCESSIONED_MONTH", false));
    entry_deaccessioned_year->SetValue(db_get_field(data, "DEACCESSIONED_YEAR", false));
    entry_deaccessioned_to->AppendText(db_get_field(data, "DEACCESSIONED_TO", false));
    entry_collection->AppendText (db_get_field(data, "COLLECTION", false));
    entry_value->AppendText      (db_get_field(data, "VALUE", false));
    entry_price->AppendText      (db_get_field(data, "PRICE", false));
    entry_selfcollected->SetValue(db_get_field(data, "SELFCOLLECTED", false)=="1");

    entry_s1_species->AppendText(db_get_field(data, "S1_SPECIES", false));
    entry_s1_variety->AppendText(db_get_field(data, "S1_VARIETY", false));
    entry_s1_class->AppendText  (db_get_field(data, "S1_CLASS", false));
    entry_s1_chemf->AppendText  (db_get_field(data, "S1_CHEMF", false));
    entry_s1_color->AppendText  (db_get_field(data, "S1_COLOR", false));
    entry_s1_transp->SetValue   (db_get_field(data, "S1_TRANSP", false));
    entry_s1_habit->SetValue    (db_get_field(data, "S1_HABIT", false));
    entry_s1_flsw->AppendText   (db_get_field(data, "S1_FLSW", false));
    entry_s1_flmw->AppendText   (db_get_field(data, "S1_FLMW", false));
    entry_s1_fllw->AppendText   (db_get_field(data, "S1_FLLW", false));
    entry_s1_fl405->AppendText  (db_get_field(data, "S1_FL405", false));
    entry_s1_phsw->AppendText   (db_get_field(data, "S1_PHSW", false));
    entry_s1_phmw->AppendText   (db_get_field(data, "S1_PHMW", false));
    entry_s1_phlw->AppendText   (db_get_field(data, "S1_PHLW", false));
    entry_s1_ph405->AppendText  (db_get_field(data, "S1_PH405", false));
    entry_s1_tenebr->AppendText (db_get_field(data, "S1_TENEBR", false));
    entry_s1_tribo->AppendText  (db_get_field(data, "S1_TRIBO", false));

    entry_s2_species->AppendText(db_get_field(data, "S2_SPECIES", false));
    entry_s2_variety->AppendText(db_get_field(data, "S2_VARIETY", false));
    entry_s2_class->AppendText  (db_get_field(data, "S2_CLASS", false));
    entry_s2_chemf->AppendText  (db_get_field(data, "S2_CHEMF", false));
    entry_s2_color->AppendText  (db_get_field(data, "S2_COLOR", false));
    entry_s2_transp->SetValue   (db_get_field(data, "S2_TRANSP", false));
    entry_s2_habit->SetValue    (db_get_field(data, "S2_HABIT", false));
    entry_s2_flsw->AppendText   (db_get_field(data, "S2_FLSW", false));
    entry_s2_flmw->AppendText   (db_get_field(data, "S2_FLMW", false));
    entry_s2_fllw->AppendText   (db_get_field(data, "S2_FLLW", false));
    entry_s2_fl405->AppendText  (db_get_field(data, "S2_FL405", false));
    entry_s2_phsw->AppendText   (db_get_field(data, "S2_PHSW", false));
    entry_s2_phmw->AppendText   (db_get_field(data, "S2_PHMW", false));
    entry_s2_phlw->AppendText   (db_get_field(data, "S2_PHLW", false));
    entry_s2_ph405->AppendText  (db_get_field(data, "S2_PH405", false));
    entry_s2_tenebr->AppendText (db_get_field(data, "S2_TENEBR", false));
    entry_s2_tribo->AppendText  (db_get_field(data, "S2_TRIBO", false));

    entry_s3_species->AppendText(db_get_field(data, "S3_SPECIES", false));
    entry_s3_variety->AppendText(db_get_field(data, "S3_VARIETY", false));
    entry_s3_class->AppendText  (db_get_field(data, "S3_CLASS", false));
    entry_s3_chemf->AppendText  (db_get_field(data, "S3_CHEMF", false));
    entry_s3_color->AppendText  (db_get_field(data, "S3_COLOR", false));
    entry_s3_transp->SetValue   (db_get_field(data, "S3_TRANSP", false));
    entry_s3_habit->SetValue    (db_get_field(data, "S3_HABIT", false));
    entry_s3_flsw->AppendText   (db_get_field(data, "S3_FLSW", false));
    entry_s3_flmw->AppendText   (db_get_field(data, "S3_FLMW", false));
    entry_s3_fllw->AppendText   (db_get_field(data, "S3_FLLW", false));
    entry_s3_fl405->AppendText  (db_get_field(data, "S3_FL405", false));
    entry_s3_phsw->AppendText   (db_get_field(data, "S3_PHSW", false));
    entry_s3_phmw->AppendText   (db_get_field(data, "S3_PHMW", false));
    entry_s3_phlw->AppendText   (db_get_field(data, "S3_PHLW", false));
    entry_s3_ph405->AppendText  (db_get_field(data, "S3_PH405", false));
    entry_s3_tenebr->AppendText (db_get_field(data, "S3_TENEBR", false));
    entry_s3_tribo->AppendText  (db_get_field(data, "S3_TRIBO", false));

    entry_s4_species->AppendText(db_get_field(data, "S4_SPECIES", false));
    entry_s4_variety->AppendText(db_get_field(data, "S4_VARIETY", false));
    entry_s4_class->AppendText  (db_get_field(data, "S4_CLASS", false));
    entry_s4_chemf->AppendText  (db_get_field(data, "S4_CHEMF", false));
    entry_s4_color->AppendText  (db_get_field(data, "S4_COLOR", false));
    entry_s4_transp->SetValue   (db_get_field(data, "S4_TRANSP", false));
    entry_s4_habit->SetValue    (db_get_field(data, "S4_HABIT", false));
    entry_s4_flsw->AppendText   (db_get_field(data, "S4_FLSW", false));
    entry_s4_flmw->AppendText   (db_get_field(data, "S4_FLMW", false));
    entry_s4_fllw->AppendText   (db_get_field(data, "S4_FLLW", false));
    entry_s4_fl405->AppendText  (db_get_field(data, "S4_FL405", false));
    entry_s4_phsw->AppendText   (db_get_field(data, "S4_PHSW", false));
    entry_s4_phmw->AppendText   (db_get_field(data, "S4_PHMW", false));
    entry_s4_phlw->AppendText   (db_get_field(data, "S4_PHLW", false));
    entry_s4_ph405->AppendText  (db_get_field(data, "S4_PH405", false));
    entry_s4_tenebr->AppendText (db_get_field(data, "S4_TENEBR", false));
    entry_s4_tribo->AppendText  (db_get_field(data, "S4_TRIBO", false));

    entry_radioact->AppendText(db_get_field(data, "RADIOACTIVITY", false));
    entry_description->AppendText(db_get_field(data, "DESCRIPTION", false));
    entry_notes->AppendText(db_get_field(data, "NOTES", false));
    entry_owners->AppendText(db_get_field(data, "OWNERS", false));

    return;
}

