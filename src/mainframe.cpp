

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/richtext/richtextctrl.h>
#include <wx/aboutdlg.h>

#include "mainframe.h"
#include "addmodframe.h"
#include "genreportframe.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_ExportCSV,        MainFrame::export_csv)
    EVT_MENU(ID_ImportCSV,        MainFrame::import_csv)
    EVT_MENU(ID_NewMineral,       MainFrame::OnNewMineral)
    EVT_MENU(ID_ModifyMineral,    MainFrame::OnModifyMineral)
    EVT_MENU(ID_DuplicateMineral, MainFrame::OnDuplicateMineral)
    EVT_MENU(ID_DeleteMineral,    MainFrame::OnDeleteMineral)
    EVT_MENU(ID_GenReport,        MainFrame::OnGenReport)
    EVT_MENU(wxID_OPEN,  MainFrame::OnOpen)
    EVT_MENU(wxID_SAVE,  MainFrame::OnSave)
    EVT_MENU(wxID_CLOSE, MainFrame::OnClose)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_HELP,  MainFrame::OnHelp)
    EVT_LISTBOX(ID_SelectMineral, MainFrame::OnSelectMineral)
    EVT_TEXT(ID_SearchMineral,      MainFrame::populate_listbox_evt_species)
    EVT_RADIOBOX(ID_OrderByMineral, MainFrame::populate_listbox_evt_species)
    EVT_CHOICE(ID_FilterCountry,    MainFrame::populate_listbox_evt_country)
    EVT_CHOICE(ID_FilterState,      MainFrame::populate_listbox_evt_state)
    EVT_CHOICE(ID_FilterSpecies,    MainFrame::populate_listbox_evt_species)
    EVT_TEXT_URL(wxID_ANY, MainFrame::OnURL)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size) {
    /* Initialize DB */
    db = NULL;
    db_file_path = "";
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxJPEGHandler);
    wxImage::AddHandler(new wxGIFHandler);
    wxImage::AddHandler(new wxTIFFHandler);
    /* Menu Bar */
    menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN);
    menuFile->Append(wxID_SAVE);
    menuFile->Append(wxID_CLOSE);
    menuFile->Append(ID_ImportCSV, "&Import CSV", "Import mineral database from a CSV file");
    menuFile->Append(ID_ExportCSV, "&Export CSV", "Export mineral database as CSV file");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    menuMineral = new wxMenu;
    menuMineral->Append(ID_NewMineral, "&Add", "Add a new mineral to the database");
    menuMineral->Append(ID_ModifyMineral, "&Modify", "Modify the selected mineral");
    menuMineral->Append(ID_DuplicateMineral, "&Duplicate", "Duplicate the selected mineral");
    menuMineral->Append(ID_DeleteMineral, "&Delete", "Delete the selected mineral");
    menuMineral->AppendSeparator();
    menuMineral->AppendCheckItem(ID_HIDEVALUE, "&Hide mineral value");
    menuMineral->Append(ID_GenReport, "&Generate printable report");
    menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    menuHelp->Append(wxID_HELP, "&Read the manual online", "Open the MineralApp manual on the browser");
    menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File" );
    menuBar->Append(menuMineral, "&Mineral" );
    menuBar->Append(menuHelp, "&Help" );
    SetMenuBar( menuBar );
    /* Status Bar */
    CreateStatusBar();
    SetStatusText("Welcome to MineralApp!" );
    /* ListBox */
    mineral_listbox = new wxListBox(this, ID_SelectMineral);
    /* Search entry */
    wxStaticText *mineral_search_label = new wxStaticText(this, -1, "Filter:");
    mineral_search = new wxTextCtrl(this, ID_SearchMineral, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    /* Order by */
    wxStaticText *mineral_orderby_label = new wxStaticText(this, -1, "Order by:");
    wxArrayString orderby_choices;
    orderby_choices.Add("Unique ID");
    orderby_choices.Add("Name");
    mineral_orderby = new wxRadioBox(this, ID_OrderByMineral, wxEmptyString, wxDefaultPosition, wxDefaultSize, orderby_choices, 2, wxRA_HORIZONTAL);
    /* Country filter */
    wxStaticText *mineral_country_label = new wxStaticText(this, -1, "Country:");
    mineral_country = new wxChoice(this, ID_FilterCountry, wxDefaultPosition, wxDefaultSize, 0, NULL);
    /* State filter */
    wxStaticText *mineral_state_label = new wxStaticText(this, -1, "State:");
    mineral_state = new wxChoice(this, ID_FilterState, wxDefaultPosition, wxDefaultSize, 0, NULL);
    /* Species filter */
    wxStaticText *mineral_species_label = new wxStaticText(this, -1, "Species:");
    mineral_species = new wxChoice(this, ID_FilterSpecies, wxDefaultPosition, wxDefaultSize, 0, NULL);
    /* Grid sizer */
    wxFlexGridSizer *leftgrid = new wxFlexGridSizer(2,0,0);
    leftgrid->AddGrowableCol(1,1);
    leftgrid->Add(mineral_search_label,  0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_search,        1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_country_label, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_country,       1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_state_label,   0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_state,         1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_species_label, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_species,       1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_orderby_label, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    leftgrid->Add(mineral_orderby,       1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    /* Viewbox */
    mineral_view = new wxRichTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY, wxDefaultValidator, wxTextCtrlNameStr);
    /* Sizers */
    wxBoxSizer *hsizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *leftvsizer = new wxBoxSizer(wxVERTICAL);
    leftvsizer->Add(mineral_listbox, 1, wxEXPAND | wxALL, 5);
    leftvsizer->Add(leftgrid, 0, wxEXPAND | wxALL, 5);
    hsizer->Add(leftvsizer, 0, wxEXPAND | wxALL, 5);
    hsizer->Add(mineral_view, 1, wxEXPAND | wxALL, 5);
    SetSizerAndFit(hsizer);
    /* Read config file */
    read_config();
}

void MainFrame::OnOpen(wxCommandEvent& event) {
    /* Check if some db is already opened and warn the user */
    if (db) {
        wxMessageDialog *dial = new wxMessageDialog(NULL, "You have already an open database. Do you want to discard it and open a new one?", "Question", wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
        if (dial->ShowModal()!=wxID_YES) {
            return;
        }
    }
    /* Get the filename of the db to read */
    wxFileDialog openFileDialog(this, "Open database", wxEmptyString, "minerals.sqlite3",  "Database files (*.sqlite3)|*.sqlite3", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal()==wxID_CANCEL) {
        return;
    }
    mineral_listbox->Clear();
    mineral_view->Clear();
    std::string fname = openFileDialog.GetPath().ToStdString();
    std::string errmsg;
    db_open(&db, fname, &errmsg);
    if (errmsg.size()>0) {
        wxLogMessage(wxString(errmsg));
    }
    db_file_path = fs::path(fname);
    write_config();
    update_gui();
}

void MainFrame::OnSave(wxCommandEvent& event) {
    if (!db) {
        wxLogMessage("Nothing to save!");
        return;
    }
    wxFileDialog saveFileDialog(this, "Save database", wxEmptyString, "minerals.sqlite3",  "Database files (*.sqlite3)|*.sqlite3", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    std::string fname = saveFileDialog.GetPath().ToStdString();
    std::string errmsg;
    db_save(db, fname, &errmsg);
    if (errmsg.size()>0) {
        wxLogMessage(wxString(errmsg));
    }
    db_file_path = fs::path(fname);
    write_config();
}

void MainFrame::OnClose(wxCommandEvent& event) {
    std::string msg = "You are going to close the current database. If you did not save it any edits will be lost. This operation cannot be undone.";
    wxMessageDialog dial(this, msg, "Are you sure you want to close the current database??", wxYES_NO | wxCANCEL | wxNO_DEFAULT);
    if (dial.ShowModal() != wxID_YES) return;
    std::string errmsg;
    db_close(db, &errmsg);
    if (errmsg.size()>0) {
        wxLogMessage(wxString(errmsg));
        return;
    }
    db = NULL;
    db_file_path = "";
    mineral_listbox->Clear();
    mineral_view->Clear();
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName("MineralApp");
    info.SetVersion(VERSION);
    info.SetDescription( ""
        "A small and simple application to create a database of "
        "your minerals. You can add your mineral collection, storing any details "
        "you are interested in, helping you (hopefully...) to keep your mineral "
        "collection well organized!");
    info.SetCopyright("(C) 2019-2023 Simone Conti <3dz2.com/mineralapp/>");
    wxAboutBox(info);
}

void MainFrame::OnHelp(wxCommandEvent& event) {
    wxLaunchDefaultBrowser("https://3dz2.com/mineralapp/manual/");
}

void MainFrame::OnURL(wxTextUrlEvent& event) {
    mineral_view->SetCaretPosition(0);
    mineral_view->SetDefaultStyleToCursorStyle();
    wxLaunchDefaultBrowser(event.GetString());
}

void MainFrame::OnNewMineral(wxCommandEvent& event) {
    std::string errmsg;
    if (!db) {
        db_initialize(&db, &errmsg);
    }
    if (!db or errmsg.size()>0) {
        wxMessageBox("DB initialization failed! Sorry, try to close everything and retry...");
        return;
    }
    AddModFrame *frame = new AddModFrame(this, "Add new mineral", db, -1);
    frame->Show();
}

int MainFrame::get_minid_from_listbox(bool warn) {
    int minid;
    int selected = mineral_listbox->GetSelection();
    if (selected==wxNOT_FOUND) {
        if (warn) wxLogMessage("Please, select a mineral from the left panel.");
        return -1;
    }
    wxString label = mineral_listbox->GetString(selected);
    int ndxi = label.rfind('[');
    int ndxf = label.rfind(']');
    int ret = sscanf(label.substr(ndxi,ndxf).c_str(), "[%d]", &minid);
    if (ret!=1) {
        if (warn) wxLogMessage("Please, select a mineral from the left panel.");
        return -1;
    }
    return minid;
}

void MainFrame::OnModifyMineral(wxCommandEvent& event) {
    int minid = get_minid_from_listbox();
    if (minid<0) return;
    AddModFrame *frame = new AddModFrame(this, "Add new mineral", db, minid);
    frame->Show();
}


void MainFrame::OnDuplicateMineral(wxCommandEvent& event) {
    int minid = get_minid_from_listbox();
    if (minid<0) return;
    std::string errmsg;
    db_duplicate_mineral(db, minid, &errmsg);
    if (errmsg.size()>0) {
        wxLogMessage(wxString("Duplicate failed: ") + errmsg);
    } else {
        update_gui();
        wxLogMessage("Mineral duplicated!");
    }
    return;
}

void MainFrame::OnDeleteMineral(wxCommandEvent& event) {
    int minid = get_minid_from_listbox();
    if (minid<0) return;
    std::string msg = "You are going to delete mineral ID " + std::to_string(minid) +  ". This operation cannot be undone.";
    wxMessageDialog dial(this, msg, "Are you sure you want to delete this mineral?", wxYES_NO | wxCANCEL | wxNO_DEFAULT);
    if (dial.ShowModal() != wxID_YES) return;
    std::string errmsg;
    db_delete_mineral(db, minid, &errmsg);
    if (errmsg.size()>0) {
        wxLogMessage(wxString(errmsg));
        return;
    }
    update_gui();
    mineral_view->Clear();
}

void MainFrame::OnSelectMineral(wxCommandEvent& event) {
    int minid = get_minid_from_listbox();
    if (minid<0) return;
    draw_mineral_view(minid);
    return;
}

void MainFrame::draw_mineral_view(int minid) {
    mineral_view->Clear();
    if (minid<0) {
        return;
    }

    std::string errmsg = "";
    std::vector<std::string> data = db_get_data(db, minid, &errmsg);

    /* Start formatting */
    wxRichTextCtrl *r = mineral_view;

    /* Set monospace */
    wxFont monospace = wxFont(wxFontInfo().FaceName("Andale Mono"));
    if (!monospace.IsFixedWidth()) {
        monospace = wxFont(wxFontInfo().FaceName("Consolas"));
    }
    r->BeginFont(monospace);

    /* Make a style suitable for showing a URL */
    wxRichTextAttr urlStyle;
    urlStyle.SetTextColour(*wxBLUE);
    urlStyle.SetFontUnderlined(true);

    /* Title */
    r->BeginFontSize(16);
    r->WriteText(db_get_field(data, "NAME"));
    r->EndFontSize();
    r->Newline();
    r->Newline();
    
    /* ID */
    r->BeginBold();
    r->WriteText("Catalog Number   : ");
    r->EndBold();
    r->WriteText(db_get_field(data, "ID"));
    r->Newline();

    /* minID */
    wxString mindat_id = db_get_field(data, "MINID");
    r->BeginBold();
    r->WriteText("minID            : ");
    r->EndBold();
    if (mindat_id.size()>0) {
        r->WriteText(mindat_id);
        r->WriteText(" (");
        r->BeginStyle(urlStyle);
        r->BeginURL(wxString("https://www.mindat.org/")+mindat_id);
        r->WriteText("See on MINDAT");
        r->EndURL();
        r->EndStyle();
        r->WriteText(")");
    }
    r->Newline();

    /* Locality */
    wxString locality = db_get_field(data, "LOCALITY");
    wxString locid = db_get_field(data, "LOCALITY_ID");
    r->BeginBold();
    r->WriteText("Locality         : ");
    r->EndBold();
    if (!locality.empty()) r->WriteText(locality);
    if (!locid.empty()) {
        r->WriteText(" (");
        r->BeginStyle(urlStyle);
        r->BeginURL(wxString("https://www.mindat.org/loc-")+locid+".html");
        r->WriteText("See on MINDAT");
        r->EndURL();
        r->EndStyle();
        r->WriteText(")");
    }
    r->Newline();
    
    /* Size, weight, acquisition, collection value */
    wxString size = db_get_fmt_size(data);
    wxString weight = db_get_fmt_weight(data);
    wxString acquisition = db_get_fmt_acquisition(data);
    wxString deaccessioned = db_get_fmt_deaccessioned(data);
    wxString collection = db_get_field(data, "COLLECTION");
    wxString value = db_get_fmt_value(data, menuMineral->IsChecked(ID_HIDEVALUE));
    r->BeginBold(); r->WriteText("Size             : "); r->EndBold(); if (size.length()>0) r->WriteText(size); r->Newline();
    r->BeginBold(); r->WriteText("Weight           : "); r->EndBold(); if (weight.length()>0) r->WriteText(weight); r->Newline();
    r->BeginBold(); r->WriteText("Acquisition      : "); r->EndBold(); if (acquisition.length()>0) r->WriteText(acquisition); r->Newline();
    r->BeginBold(); r->WriteText("Collection       : "); r->EndBold(); if (collection.length()>0) r->WriteText(collection); r->Newline();
    r->BeginBold(); r->WriteText("Estimated Value  : "); r->EndBold(); if (value.length()>0) r->WriteText(value); r->Newline();
    if (deaccessioned.size()>0) {
        r->BeginBold(); r->WriteText("Deaccessioned    : "); r->EndBold(); r->WriteText(deaccessioned); r->Newline();
    }
    r->Newline();

    /* Table of species */
    write_table_row("Species          ", data, "SPECIES");
    write_table_row("Variety          ", data, "VARIETY");
    write_table_row("Class            ", data, "CLASS");
    write_table_row_chemf("Chem. Formula    ", data);
    write_table_row("Color            ", data, "COLOR");
    write_table_row("Transparency     ", data, "TRANSP");
    write_table_row("Habit            ", data, "HABIT");
    write_table_row("Fluorescence SW  ", data, "FLSW");
    write_table_row("Fluorescence MW  ", data, "FLMW");
    write_table_row("Fluorescence LW  ", data, "FLLW");
    write_table_row("Fluor. 405nm     ", data, "FL405");
    write_table_row("Phosphor. SW     ", data, "PHSW");
    write_table_row("Phosphor. MW     ", data, "PHMW");
    write_table_row("Phosphor. LW     ", data, "PHLW");
    write_table_row("Phosphor. 405nm  ", data, "PH405");
    write_table_row("Tenebrescence    ", data, "TENEBR");
    write_table_row("Triboluminescence", data, "TRIBO");
    write_link_row(data);
    r->Newline();

    /* Radioactivity */
    wxString radioact = db_get_field(data, "RADIOACTIVITY");
    if (!radioact.empty()) {
        r->BeginBold(); r->WriteText("Radioactivity    : "); r->EndBold();
        r->WriteText(radioact); r->Newline();
        r->Newline();
    }

    /* Comments */
    wxString description = db_get_field(data, "DESCRIPTION");
    if (!description.empty()) {
        r->BeginBold(); r->WriteText("Description: "); r->EndBold(); r->Newline();
        r->WriteText(description); r->Newline();
        r->Newline();
    }
    wxString notes = db_get_field(data, "NOTES");
    if (!notes.empty()) {
        r->BeginBold(); r->WriteText("Notes: "); r->EndBold(); r->Newline();
        r->WriteText(notes); r->Newline();
        r->Newline();
    }
    wxString owners = db_get_field(data, "OWNERS");
    if (!owners.empty()) {
        r->BeginBold(); r->WriteText("Previous owners: "); r->EndBold(); r->Newline();
        r->WriteText(owners); r->Newline();
        r->Newline();
    }

    /* Data */
    ReadData(db_get_field(data, "ID"));

    r->EndFont();

    return;
}

static inline int guess_column_width(std::vector<std::string> data, int column) {
    std::vector<std::string> fields = { "SPECIES", "VARIETY", "CLASS", "COLOR", "CHEMF", "HABIT", "TRANSP", "FLSW", \
        "FLMW", "FLLW", "FL405", "PHSW", "PHMW", "PHLW", "PH405", "TENEBR", "TRIBO" };
    int ln;
    int length = 16;
    for (auto field : fields) {
        ln = db_get_field(data, std::string("S")+std::to_string(column)+"_"+field).length();
        length = std::max(length, ln);
    }
    return length+1;
}

void MainFrame::write_table_row(wxString name, std::vector<std::string> data, std::string field) {
    wxRichTextCtrl *r = mineral_view;
    wxString s1 = db_get_field(data, std::string("S1_")+field);
    wxString s2 = db_get_field(data, std::string("S2_")+field);
    wxString s3 = db_get_field(data, std::string("S3_")+field);
    wxString s4 = db_get_field(data, std::string("S4_")+field);
    if (s1=="No" || s1=="no") s1=wxString("");
    if (s2=="No" || s2=="no") s2=wxString("");
    if (s3=="No" || s3=="no") s3=wxString("");
    if (s4=="No" || s4=="no") s4=wxString("");
    int l1 = s1.length();
    int l2 = s2.length();
    int l3 = s3.length();
    int l4 = s4.length();
    if (l1+l2+l3+l4>0) {
        r->BeginBold(); r->WriteText(name+": "); r->EndBold();
        r->WriteText(s1+std::string(guess_column_width(data, 1)-s1.length(), ' '));
        if (l2+l3+l4>0) r->WriteText(s2+std::string(guess_column_width(data, 2)-s2.length(), ' '));
        if (l3+l4>0) r->WriteText(s3+std::string(guess_column_width(data, 3)-s3.length(), ' '));
        if (l4>0) r->WriteText(s4+std::string(guess_column_width(data, 4)-s4.length(), ' '));
        r->Newline();
    }
}

/*
    All the subsup mess here is because "monospace" is not monospace when subscripts or
    superscripts are used. These occupy approximately (?) 1/3 of the space normally
    used.
*/
static int write_chemf(wxRichTextCtrl *r, wxString chemfwx, int *subsup) {

    std::string chemf = chemfwx.ToStdString();
    strip_unicode(chemf);

    if (chemf=="") return 0;

    int inserted = 0;
    bool subscript = true;

    if (chemf.find(" . ")!=std::string::npos) {
        chemf = chemf.replace(chemf.find(" . "), 3, "*");
    }
    for (size_t n=0; n<chemf.length(); n++) {
        char tchar = chemf[n];
        char nchar = 0;
        if (n<chemf.length()-1) {
            nchar = chemf[n+1];
        }
        if (tchar=='+' || tchar=='-' || nchar=='+' || nchar=='-') {
            r->WriteText(wxString(1,tchar));
            r->SetSelection(r->GetCaretPosition(), r->GetCaretPosition()+1);
            r->ApplyTextEffectToSelection(wxTEXT_ATTR_EFFECT_SUPERSCRIPT);
            r->SetSelection(0,0);
            (*subsup)++;
        } else if ((isdigit(tchar) || tchar=='.') && subscript) {
            r->WriteText(wxString(1,tchar));
            r->SetSelection(r->GetCaretPosition(), r->GetCaretPosition()+1);
            r->ApplyTextEffectToSelection(wxTEXT_ATTR_EFFECT_SUBSCRIPT);
            r->SetSelection(0,0);
            (*subsup)++;
        } else if (tchar=='*') {
            //r->WriteText("\u00b7");   /* Does not work on Ubuntu :( */
            r->WriteText(wxString(1,'.'));
            r->SetSelection(r->GetCaretPosition(), r->GetCaretPosition()+1);
            r->ApplyTextEffectToSelection(wxTEXT_ATTR_EFFECT_SUPERSCRIPT);
            r->SetSelection(0,0);
            subscript = false;
            (*subsup)++;
        } else {
            r->WriteText(wxString(1,tchar));
            if (isalpha(tchar)) {
                subscript = true;
            }
        }
        inserted++;
    }

    return inserted;
}


void MainFrame::write_table_row_chemf(wxString name, std::vector<std::string> data) {
    wxRichTextCtrl *r = mineral_view;
    wxString s1 = db_get_field(data, "S1_CHEMF");
    wxString s2 = db_get_field(data, "S2_CHEMF");
    wxString s3 = db_get_field(data, "S3_CHEMF");
    wxString s4 = db_get_field(data, "S4_CHEMF");
    if (s1=="No" || s1=="no") s1=wxString("");
    if (s2=="No" || s2=="no") s2=wxString("");
    if (s3=="No" || s3=="no") s3=wxString("");
    if (s4=="No" || s4=="no") s4=wxString("");
    int l1 = s1.length();
    int l2 = s2.length();
    int l3 = s3.length();
    int l4 = s4.length();
    int subsup=0;
    if (l1+l2+l3+l4>0) {
        r->BeginBold(); r->WriteText(name+": "); r->EndBold();
        l1 = write_chemf(r, s1, &subsup);
        r->WriteText(std::string(guess_column_width(data, 1)-l1+(int)(subsup/3), ' '));
        subsup-=3*(int)(subsup/3);
        if (l2+l3+l4>0) {
            l2 = write_chemf(r, s2, &subsup);
            r->WriteText(std::string(guess_column_width(data, 2)-l2+(int)(subsup/3), ' '));
            subsup-=3*(int)(subsup/3);
        }
        if (l3+l4>0) {
            l3 = write_chemf(r, s3, &subsup);
            r->WriteText(std::string(guess_column_width(data, 3)-l3+(int)(subsup/3), ' '));
            subsup-=3*(int)(subsup/3);
        }
        if (l4>0) {
            l4 = write_chemf(r, s4, &subsup);
            r->WriteText(std::string(guess_column_width(data, 4)-l4+(int)(subsup/3), ' '));
            subsup-=3*(int)(subsup/3);
        }
        r->Newline();
    }
}


void MainFrame::write_link_row(std::vector<std::string> data) {
    wxRichTextAttr urlStyle;
    urlStyle.SetTextColour(*wxBLUE);
    urlStyle.SetFontUnderlined(true);
    wxRichTextCtrl *r = mineral_view;
    wxString s1 = db_get_field(data, "S1_VARIETY");
    wxString s2 = db_get_field(data, "S2_VARIETY");
    wxString s3 = db_get_field(data, "S3_VARIETY");
    wxString s4 = db_get_field(data, "S4_VARIETY");
    if (s1=="No" || s1=="no") s1=wxString("");
    if (s2=="No" || s2=="no") s2=wxString("");
    if (s3=="No" || s3=="no") s3=wxString("");
    if (s4=="No" || s4=="no") s4=wxString("");
    if (s1.empty()) s1 = db_get_field(data, "S1_SPECIES");
    if (s2.empty()) s2 = db_get_field(data, "S2_SPECIES");
    if (s3.empty()) s3 = db_get_field(data, "S3_SPECIES");
    if (s4.empty()) s4 = db_get_field(data, "S4_SPECIES");
    if (s1=="No" || s1=="no") s1=wxString("");
    if (s2=="No" || s2=="no") s2=wxString("");
    if (s3=="No" || s3=="no") s3=wxString("");
    if (s4=="No" || s4=="no") s4=wxString("");
    int l1 = s1.length();
    int l2 = s2.length();
    int l3 = s3.length();
    int l4 = s4.length();
    std::string s1p = url_encode(s1.ToStdString());
    std::string s2p = url_encode(s2.ToStdString());
    std::string s3p = url_encode(s3.ToStdString());
    std::string s4p = url_encode(s4.ToStdString());
    if (l1+l2+l3+l4>0) {
        r->BeginBold(); r->WriteText("                   "); r->EndBold();
        r->BeginStyle(urlStyle); r->BeginURL(wxString("http://www.mindat.org/show.php?name=")+s1p);r->WriteText("M");r->EndURL();r->EndStyle();
        r->WriteText(" ");
        r->BeginStyle(urlStyle); r->BeginURL(wxString("https://rruff.info/")+s1p);r->WriteText("R");r->EndURL();r->EndStyle();
        r->WriteText(std::string(guess_column_width(data, 1)-3, ' '));
        if (l2+l3+l4>0) {
            r->BeginStyle(urlStyle);r->BeginURL(wxString("http://www.mindat.org/show.php?name=")+s2p);r->WriteText("M");r->EndURL();r->EndStyle();
            r->WriteText(" ");
            r->BeginStyle(urlStyle); r->BeginURL(wxString("https://rruff.info/")+s2p);r->WriteText("R");r->EndURL();r->EndStyle();
            r->WriteText(std::string(guess_column_width(data, 2)-3, ' '));
        }
        if (l3+l4>0) {
            r->BeginStyle(urlStyle);r->BeginURL(wxString("http://www.mindat.org/show.php?name=")+s3p);r->WriteText("M");r->EndURL();r->EndStyle();
            r->WriteText(" ");
            r->BeginStyle(urlStyle); r->BeginURL(wxString("https://rruff.info/")+s3p);r->WriteText("R");r->EndURL();r->EndStyle();
            r->WriteText(std::string(guess_column_width(data, 3)-3, ' '));
        }
        if (l4>0) {
            r->BeginStyle(urlStyle);r->BeginURL(wxString("http://www.mindat.org/show.php?name=")+s4p);r->WriteText("M");r->EndURL();r->EndStyle();
            r->WriteText(" ");
            r->BeginStyle(urlStyle); r->BeginURL(wxString("https://rruff.info/")+s4p);r->WriteText("R");r->EndURL();r->EndStyle();
            r->WriteText(std::string(guess_column_width(data, 4)-3, ' '));
        }
        r->Newline();
    }
}


void MainFrame::ReadData(std::string uid) {

    if (db_file_path.empty()) return;

    std::vector<fs::path> files = db_get_datafile_list(db_file_path, uid);

    wxRichTextAttr urlStyle;
    urlStyle.SetTextColour(*wxBLUE);
    urlStyle.SetFontUnderlined(true);

    std::vector<std::string> formats = { ".png", ".jpg", ".jpeg", ".gif", ".tiff", ".tif" };
    wxRichTextCtrl *r = mineral_view;
    int width, height;
    float scale;

    r->BeginBold(); r->WriteText("Figures & Data :"); r->EndBold();
    r->Newline();
    r->Newline();

    for (const auto & path : files) {
        r->WriteText("    ");
        std::string ext = path.extension().string();
        if (std::find(formats.begin(), formats.end(), ext) != formats.end()) {
            wxImage image = wxImage(wxString(path.string()));
            width = image.GetWidth();
            height = image.GetHeight();
            if (width>400 || height>400) {
                scale = 400.0/std::max(width, height);
                image.Rescale(int(width*scale), int(height*scale));
            }
            r->WriteImage(image);
            r->WriteText("    ");
        } else if (fs::is_directory(path)) {
            r->WriteText(wxString("directory: "));
        } else {
            r->WriteText(wxString("file: "));
        }
        r->BeginStyle(urlStyle);r->BeginURL(wxString("file://")+url_encode(path));r->WriteText(wxString(path.filename()));r->EndURL();r->EndStyle();
        r->WriteText(" ");
        r->Newline();
        r->Newline();
    }
}

void MainFrame::populate_listbox_evt_country(wxCommandEvent& event) {
    populate_listbox();
    populate_state_filter();
    populate_species_filter();
}

void MainFrame::populate_listbox_evt_state(wxCommandEvent& event) {
    populate_listbox();
    populate_species_filter();
}

void MainFrame::populate_listbox_evt_species(wxCommandEvent& event) {
    populate_listbox();
}

void MainFrame::populate_listbox() {

    mineral_listbox->Clear();

    std::string orderby;
    int orderby_int = mineral_orderby->GetSelection();
    if (orderby_int==1) orderby="NAME"; else orderby="ID";
    std::string searchstr = str_tolower(mineral_search->GetValue().ToStdString());

    int country_id = mineral_country->GetSelection();
    std::string country;
    if (country_id!=wxNOT_FOUND) country = mineral_country->GetString(country_id);
    if (country=="Any") country="";

    int state_id = mineral_state->GetSelection();
    std::string state;
    if (state_id!=wxNOT_FOUND) state = mineral_state->GetString(state_id);
    if (state=="Any") state="";

    std::string locality = "";
    if (country!="") {
        if (state!="") {
            locality = state + ", " + country;
        } else {
            locality = country;
        }
    } else {
        locality = "";
    }

    int species_id = mineral_species->GetSelection();
    std::string species;
    if (species_id!=wxNOT_FOUND) species = mineral_species->GetString(species_id);
    if (species=="Any") species="";

    std::string errmsg;
    std::vector<std::string> results = db_search_minerals(db, searchstr, searchstr, locality, species, orderby, &errmsg);
    if (errmsg!="") {
        wxLogMessage("error: %s", errmsg);
        return;
    }
    for (auto res : results) mineral_listbox->Append(res);

    return;
}

void MainFrame::populate_country_filter() {
    mineral_country->Clear();
    mineral_country->Append("Any");
    std::string errmsg = "";
    std::vector<std::string> countries = db_get_country_list(db, &errmsg);
    for(const auto& value: countries) {
        mineral_country->Append(value);
    }
    return;
}

void MainFrame::populate_state_filter() {
    int country_id = mineral_country->GetSelection();
    std::string country;
    if (country_id!=wxNOT_FOUND) country = mineral_country->GetString(country_id);
    if (country=="Any") country="";
    mineral_state->Clear();
    mineral_state->Append("Any");
    std::string errmsg = "";
    std::vector<std::string> states = db_get_state_list(db, country, &errmsg);
    for(const auto& value: states) {
        mineral_state->Append(value);
    }
    return;
}

void MainFrame::populate_species_filter() {

    int country_id = mineral_country->GetSelection();
    std::string country;
    if (country_id!=wxNOT_FOUND) country = mineral_country->GetString(country_id);
    if (country=="Any") country="";

    int state_id = mineral_state->GetSelection();
    std::string state;
    if (state_id!=wxNOT_FOUND) state = mineral_state->GetString(state_id);
    if (state=="Any") state="";

    std::string locality = "";
    if (country!="") {
        if (state!="") {
            locality = state + ", " + country;
        } else {
            locality = country;
        }
    } else {
        locality = "";
    }

    mineral_species->Clear();
    mineral_species->Append("Any");
    std::string errmsg = "";
    std::vector<std::string> species = db_get_species_list(db, locality, &errmsg);
    for(const auto& value: species) {
        mineral_species->Append(value);
    }
    return;
}


void MainFrame::update_gui() {
    populate_country_filter();
    populate_state_filter();
    populate_species_filter();
    populate_listbox();
    return;
}

void MainFrame::import_csv(wxCommandEvent& event) {
    std::string errmsg;
    /* Check if some db is already opened and warn the user */
    if (db) {
        wxMessageDialog *dial = new wxMessageDialog(NULL, "You have already an open database. By importing from a CSV file any duplicate mineral id will overwtie existing ones. Do you want to continue?", "Question", wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
        if (dial->ShowModal()!=wxID_YES) {
            return;
        }
    } else {
        db_initialize(&db, &errmsg);
        if (errmsg.size()>0) {
            wxLogMessage(wxString(errmsg));
            return;
        }
    }

    /* Get the filename of the db to read */
    wxFileDialog openFileDialog(this, "Import CSV file", wxEmptyString, "minerals.csv",  "CSV files (*.csv)|*.csv", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal()==wxID_CANCEL) {
        return;
    }
    wxString fname = openFileDialog.GetPath();

    bool success = db_csv_import(db, fname.ToStdString(), &errmsg);
    if (!success) {
        wxLogMessage(wxString("Import failed! ") + errmsg);
    }
    update_gui();
}

void MainFrame::export_csv(wxCommandEvent& event) {
    if (!db) {
        wxLogMessage("Nothing to save!");
        return;
    }
    wxFileDialog saveFileDialog(this, "Export CSV file", wxEmptyString, "minerals.csv",  "CSV files (*.csv)|*.csv", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    std::string errmsg = "";
    bool success = db_csv_export(db, saveFileDialog.GetPath().ToStdString(), &errmsg);
    if (!success) {
        wxLogMessage(wxString(errmsg));
    }
}

fs::path MainFrame::get_config_dirname() {
    fs::path configdir;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    configdir = fs::path(std::getenv("APPDATA")) / "mineralapp";
    #else
    configdir = fs::path(std::getenv("HOME")) / ".mineralapp";
    #endif
    return configdir;
}

void MainFrame::read_config() {
    fs::path configdir = get_config_dirname();
    if (configdir.empty()) return;
    if (!fs::exists(configdir)) return;
    std::ifstream configfile;
    configfile.open(configdir/"config.txt");
    std::string last_open;
    std::string line;
    while (std::getline(configfile, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                if (key.compare(std::string("last_open"))==0) {
                    last_open = value;
                }
            }
        }
    }
    configfile.close();
    if (!last_open.empty()) {
        std::string errmsg;
        db_open(&db, last_open, &errmsg);
        if (errmsg.size()>0) {
            wxLogMessage(wxString(errmsg));
        }
        db_file_path = fs::path(last_open);
        update_gui();
    }
}


void MainFrame::write_config() {
    fs::path configdir = get_config_dirname();
    if (configdir.empty()) return;
    if (!fs::exists(configdir)) fs::create_directory(configdir);
    std::ofstream configfile;
    configfile.open(fs::path(configdir)/"config.txt");
    configfile << "last_open=" << db_file_path << std::endl;
    configfile.close();
}

void MainFrame::OnGenReport(wxCommandEvent& event) {
    int minid = get_minid_from_listbox(false);
    GenReportFrame *frame = new GenReportFrame(this, "Generate printable report", db, minid, db_file_path);
    frame->Show();
}


