
#include "mineraldb.hpp"

#include <cstring>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "parsecsv.hpp"


/* Initialize an empty db */
void db_initialize(sqlite3 **db, std::string *errmsg) {
    int ret;
    char *errmsg_c;
    ret = sqlite3_open(":memory:", db);
    if (ret!=SQLITE_OK) {
        *errmsg += "Can't open database: ";
        *errmsg += sqlite3_errmsg(*db);
        return;
    }
    const char *query_minerals_create = "CREATE TABLE MINERALS (\
        ID INTEGER PRIMARY KEY, MINID TEXT, NAME TEXT NOT NULL, LOCALITY TEXT, LOCALITY_ID TEXT, LOCALITY_LATITUDE TEXT, LOCALITY_LONGITUDE TEXT, \
        LENGTH TEXT, WIDTH TEXT, HEIGHT TEXT, SIZE_UNITS TEXT, SIZE_CATEGORY TEXT, WEIGHT TEXT, WEIGHT_UNITS TEXT, \
        ACQUISITION_YEAR TEXT, ACQUISITION_MONTH TEXT, ACQUISITION_DAY TEXT, ACQUISITION_SOURCE TEXT, SELFCOLLECTED TEXT, \
        COLLECTION TEXT, VALUE TEXT, PRICE TEXT, \
        DEACCESSIONED TEXT, DEACCESSIONED_TO TEXT, DEACCESSIONED_YEAR TEXT, DEACCESSIONED_MONTH TEXT, DEACCESSIONED_DAY TEXT, \
        S1_SPECIES TEXT, S1_VARIETY TEXT, S1_CLASS TEXT, S1_CHEMF TEXT, S1_COLOR TEXT, S1_TRANSP TEXT, S1_HABIT TEXT, S1_FLSW TEXT, S1_FLMW TEXT, \
            S1_FLLW TEXT, S1_FL405 TEXT, S1_PHSW TEXT, S1_PHMW TEXT, S1_PHLW TEXT, S1_PH405 TEXT, S1_TENEBR TEXT, S1_TRIBO TEXT, \
        S2_SPECIES TEXT, S2_VARIETY TEXT, S2_CLASS TEXT, S2_CHEMF TEXT, S2_COLOR TEXT, S2_TRANSP TEXT, S2_HABIT TEXT, S2_FLSW TEXT, S2_FLMW TEXT, \
            S2_FLLW TEXT, S2_FL405 TEXT, S2_PHSW TEXT, S2_PHMW TEXT, S2_PHLW TEXT, S2_PH405 TEXT, S2_TENEBR TEXT, S2_TRIBO TEXT, \
        S3_SPECIES TEXT, S3_VARIETY TEXT, S3_CLASS TEXT, S3_CHEMF TEXT, S3_COLOR TEXT, S3_TRANSP TEXT, S3_HABIT TEXT, S3_FLSW TEXT, S3_FLMW TEXT, \
            S3_FLLW TEXT, S3_FL405 TEXT, S3_PHSW TEXT, S3_PHMW TEXT, S3_PHLW TEXT, S3_PH405 TEXT, S3_TENEBR TEXT, S3_TRIBO TEXT, \
        S4_SPECIES TEXT, S4_VARIETY TEXT, S4_CLASS TEXT, S4_CHEMF TEXT, S4_COLOR TEXT, S4_TRANSP TEXT, S4_HABIT TEXT, S4_FLSW TEXT, S4_FLMW TEXT, \
            S4_FLLW TEXT, S4_FL405 TEXT, S4_PHSW TEXT, S4_PHMW TEXT, S4_PHLW TEXT, S4_PH405 TEXT, S4_TENEBR TEXT, S4_TRIBO TEXT, \
        RADIOACTIVITY TEXT, RADIOACTIVITY_UNITS TEXT, DESCRIPTION TEXT, NOTES TEXT, OWNERS TEXT );";
    ret = sqlite3_exec(*db, query_minerals_create, NULL, 0, &errmsg_c);
    if (ret!=SQLITE_OK) {
        *errmsg += errmsg_c;
        sqlite3_free(errmsg);
        sqlite3_close(*db);
        *db=NULL;
        return;
    }
    const char *query_settings_create = "CREATE TABLE SETTINGS (VERSION_MAJOR INT, VERSION_MINOR INT)";
    ret = sqlite3_exec(*db, query_settings_create, NULL, 0, &errmsg_c);
    if (ret!=SQLITE_OK) {
        *errmsg += errmsg_c;
        sqlite3_free(errmsg);
        sqlite3_close(*db);
        *db=NULL;
        return;
    }
    const char *query_set_version = "INSERT INTO SETTINGS (VERSION_MAJOR, VERSION_MINOR) VALUES (" VERSION_MAJOR ", " VERSION_MINOR ");";
    ret = sqlite3_exec(*db, query_set_version, NULL, 0, &errmsg_c);
    if (ret!=SQLITE_OK) {
        *errmsg += errmsg_c;
        sqlite3_free(errmsg);
        sqlite3_close(*db);
        *db=NULL;
        return;
    }
    return;
}

/* Get version from db */
static int db_get_version(sqlite3 *db) {
    int major, minor;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT VERSION_MAJOR, VERSION_MINOR FROM SETTINGS", -1, &stmt, NULL);
    int ret=sqlite3_step(stmt);
    if (ret!=SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }
    major = sqlite3_column_int(stmt, 0);
    minor = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
    return major*100+minor;
}

/* Convert to string */
template <typename T> std::string to_string(const T& t) {
   std::ostringstream os;
   os<<t;
   return os.str();
}

/* Import Mineral DB version 2.0 */
static void db_import_v200(sqlite3 **newdb, sqlite3 *olddb, std::string *errmsg) {

    char u[512];
    float l, w, h;
    int ret;
    std::string size, weight;

    /* Initialize an empty new db */
    db_initialize(newdb, errmsg);
    if (errmsg->size()>0) {
        return;
    }
    /* MINID has been changed to ID */
    char *errmsg_c;
    const char *query = "ALTER TABLE MINERALS RENAME COLUMN MINID to ID";
    ret = sqlite3_exec(olddb, query, NULL, 0, &errmsg_c);
    if (ret!=SQLITE_OK) {
        *errmsg += "Import v2.0 failed! ";
        *errmsg += errmsg_c;
        sqlite3_free(errmsg_c);
        return;
    }

    std::vector<int> ids = db_get_minid_list(olddb, 0, errmsg);
    if (errmsg->size()>0) return;

    std::vector<std::string> stay_the_same = {
    "ID", "NAME", "LOCALITY", "COLLECTION", "VALUE",
    "S1_SPECIES", "S1_CLASS", "S1_CHEMF", "S1_COLOR", "S1_FLSW", "S1_FLMW", "S1_FLLW", "S1_FL405", "S1_PHSW", "S1_PHMW", "S1_PHLW", "S1_PH405", "S1_TENEBR",
    "S2_SPECIES", "S2_CLASS", "S2_CHEMF", "S2_COLOR", "S2_FLSW", "S2_FLMW", "S2_FLLW", "S2_FL405", "S2_PHSW", "S2_PHMW", "S2_PHLW", "S2_PH405", "S2_TENEBR",
    "S3_SPECIES", "S3_CLASS", "S3_CHEMF", "S3_COLOR", "S3_FLSW", "S3_FLMW", "S3_FLLW", "S3_FL405", "S3_PHSW", "S3_PHMW", "S3_PHLW", "S3_PH405", "S3_TENEBR",
    "S4_SPECIES", "S4_CLASS", "S4_CHEMF", "S4_COLOR", "S4_FLSW", "S4_FLMW", "S4_FLLW", "S4_FL405", "S4_PHSW", "S4_PHMW", "S4_PHLW", "S4_PH405", "S4_TENEBR",
    };

    for (int id : ids) {
        std::vector<std::string> olddata;
        std::vector<std::string> newdata(data_header.size());

        olddata = db_get_data(olddb, id, errmsg, 200);
        if (errmsg->size()>0) return;

        // These fields stayed the same
        for (auto field : stay_the_same) {
            newdata[db_get_field_index(field)] = olddata[db_get_field_index_200(field)];
        }

        // These fields were renamed
        newdata[db_get_field_index("LOCALITY_ID")] = olddata[db_get_field_index_200("LOCID_MNDAT")];
        newdata[db_get_field_index("ACQUISITION_SOURCE")] = olddata[db_get_field_index_200("ACQUISITION")];
        newdata[db_get_field_index("RADIOACTIVITY")] = olddata[db_get_field_index_200("RADIOACT")];
        newdata[db_get_field_index("NOTES")] = olddata[db_get_field_index_200("COMMENTS")];

        // Try to parse SIZE into LENGTH, WIDTH, HEIGHT, SIZE_UNIT
        size = olddata[db_get_field_index_200("SIZE")];
        ret = sscanf(size.c_str(), "%fx%fx%f%510[^\n]", &l, &w, &h, u);
        if (ret!=4) ret = sscanf(size.c_str(), "%f x %f x %f %510[^\n]", &l, &w, &h, u);
        if (ret==4) {
            newdata[db_get_field_index("LENGTH")] = to_string(l);
            newdata[db_get_field_index("WIDTH")] = to_string(w);
            newdata[db_get_field_index("HEIGHT")] = to_string(h);
            newdata[db_get_field_index("SIZE_UNITS")] = std::string(u);
        } else {
            newdata[db_get_field_index("SIZE_UNITS")] = size;
        }

        // Try to parse weight
        weight = olddata[db_get_field_index_200("WEIGHT")];
        ret = sscanf(weight.c_str(), "%f%510[^\n]", &w, u);
        if (ret==2) {
            newdata[db_get_field_index("WEIGHT")] = to_string(w);
            newdata[db_get_field_index("WEIGHT_UNITS")] = ((strlen(u)>1 && u[0]==' ') ? u+1 : u);
        } else {
            newdata[db_get_field_index("WEIGHT")] = weight;
        }

        // Everything done! Save the new data into the db
        db_addmod_mineral(*newdb, newdata, -1, errmsg);
    }
}


/* Open db from file */
void db_open(sqlite3 **db, std::string fname, std::string *errmsg) {

    int ret;

    /* Open the db from file into temporary db */
    sqlite3 *db_tmp;
    ret = sqlite3_open(fname.c_str(), &db_tmp);
    if (ret!=SQLITE_OK) {
        *errmsg += "Failed to open file for reading!";
        return;
    }

    /* Make an in-memory copy */
    sqlite3 *db_mem;
    ret = sqlite3_open(":memory:", &db_mem);
    if (ret!=SQLITE_OK) {
        *errmsg += "Can't create memory database: ";
        *errmsg += sqlite3_errmsg(db_mem);
        sqlite3_close(db_tmp);
        return;
    }
    sqlite3_backup *bkp;
    bkp = sqlite3_backup_init(db_mem, "main", db_tmp, "main");
    if (!bkp) {
        *errmsg += "Failed to backup file!";
        sqlite3_close(db_tmp);
        sqlite3_close(db_mem);
        return;
    }
    sqlite3_backup_step(bkp, -1);
    sqlite3_backup_finish(bkp);
    sqlite3_close(db_tmp);

    /* Get version */
    int dbversion = db_get_version(db_mem);
    if (dbversion<0) {
        *errmsg = "Couldn't get DB version from file. Cannot parse it.";
        sqlite3_close(db_mem);
        return;
    }

    /* Do the job */
    if (dbversion==200) {
        db_import_v200(db, db_mem, errmsg);
    } else {
        *db = db_mem;
    }

    return;
}


/* Save db to file */
void db_save(sqlite3 *db, std::string fname, std::string *errmsg) {
    sqlite3 *db_tmp;
    int ret = sqlite3_open(fname.c_str(), &db_tmp);
    if (ret!=SQLITE_OK) {
        *errmsg += "Failed to open file for writing!";
        return;
    }
    sqlite3_backup *bkp;
    bkp = sqlite3_backup_init(db_tmp, "main", db, "main");
    if (!bkp) {
        *errmsg += "Failed to initialize database saving! ";
        *errmsg += sqlite3_errmsg(db_tmp);
        return;
    }
    ret = sqlite3_backup_step(bkp, -1);
    if (ret != SQLITE_DONE) {
        *errmsg += "Failed to save all data to file (1)";
    }
    ret = sqlite3_backup_finish(bkp);
    if (ret != SQLITE_OK) {
        *errmsg += "Failed to save all data to file (2)";
    }
    sqlite3_close(db_tmp);
}


/* Close database */
void db_close(sqlite3 *db, std::string *errmsg) {
    int ret = sqlite3_close(db);
    if (ret!=SQLITE_OK) {
        *errmsg += "Cannot close current db. Sorry!";
    }
}


/* Delete a mineral from the database */
void db_delete_mineral(sqlite3 *db, int minid, std::string *errmsg) {
    std::string query = "DELETE FROM MINERALS WHERE ID=" + std::to_string(minid);
    char *errmsg_c;
    int ret = sqlite3_exec(db, query.c_str(), NULL, 0, &errmsg_c);
    if (ret!=SQLITE_OK) {
        *errmsg += "Delete failed! ";
        *errmsg += errmsg_c;
        sqlite3_free(errmsg_c);
    }
}


/* Duplicate the specified minid */
void db_duplicate_mineral(sqlite3 *db, int minid, std::string *errmsg) {
    std::string  query = " \
        CREATE TEMPORARY TABLE temp_table as SELECT * FROM MINERALS WHERE ID=" + std::to_string(minid) + "; \
        UPDATE temp_table SET ID=NULL; \
        INSERT INTO MINERALS SELECT * FROM temp_table; \
        DROP TABLE temp_table; \
    ";
    int ret;
    char *errmsg_c;
    ret = sqlite3_exec(db, query.c_str(), NULL, 0, &errmsg_c);
    if (ret!=SQLITE_OK) {
        *errmsg += errmsg_c;
        sqlite3_free(errmsg);
    }
    return;
}


/* Get the index of a given field from the data_header vector */
int db_get_field_index(std::string field) {
    std::vector<std::string> header = data_header;
    auto it = find(header.begin(), header.end(), field);
    if (it != header.end()) return distance(header.begin(), it);
    else return -1;
}
int db_get_field_index_200(std::string field) {
    std::vector<std::string> header = data_header_200;
    auto it = find(header.begin(), header.end(), field);
    if (it != header.end()) return distance(header.begin(), it);
    else return -1;
}

/* Get the field value from the data vector */
std::string db_get_field(std::vector<std::string> data, std::string field, bool remove_no) {
    int ndx = db_get_field_index(field);
    if (ndx>=0) {
        std::string tmp = data[ndx];
        if (remove_no && tmp=="No") return std::string();
        else return tmp;
    }
    else return "ERROR!";
}

/* Get all data relative to a minid from the db */
std::vector<std::string> db_get_data(sqlite3 *db, int minid, std::string *errmsg, int header_version) {
    std::vector<std::string> data;
    if (minid<0) {
        *errmsg += "ID is negative. It should be positive...";
        return data;
    }
    std::vector<std::string> header;
    if (header_version==200) {
        header = data_header_200;
    } else {
        header = data_header;
    }
    size_t header_size = header.size();
    std::string query = "SELECT ";
    for (size_t i=0; i<header_size; i++) {
        query += header[i];
        if (i<header_size-1) query += ", ";
    }
    query += " FROM MINERALS WHERE ID=?";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, minid);
    int ret=sqlite3_step(stmt);
    if (ret!=SQLITE_ROW) {
        *errmsg += "Impossible to find ID in database :(";
        sqlite3_finalize(stmt);
        return data;
    }
    for (size_t i=0; i<header.size(); i++) {
        const unsigned char *uc = sqlite3_column_text(stmt, i);
        std::string tmp = "";
        if (uc!=NULL) tmp=(const char*)uc;
        data.push_back(tmp);
    }
    sqlite3_finalize(stmt);
    return data;
}

/* Get the list of minid */
std::vector<int> db_get_minid_list(sqlite3 *db, int orderby, std::string *errmsg) {
    std::vector<int> minids;
    const char *query;
    if (orderby==1) {
        query = "SELECT ID FROM MINERALS ORDER BY NAME";
    } else {
        query = "SELECT ID FROM MINERALS ORDER BY ID";
    }
    int ret;
    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (ret!=SQLITE_OK) {
        *errmsg += std::string("Error: ") + sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        return minids;
    }
    while ((ret=sqlite3_step(stmt))==SQLITE_ROW) {
        const unsigned char *uc = sqlite3_column_text(stmt, 0);
        std::string tmp = "";
        if (uc!=NULL) tmp=(const char*)uc;
        int minid = -1;
        int ret = sscanf(tmp.c_str(), "%d", &minid);
        if (ret!=1) minid = -1;
        minids.push_back(minid);
    }
    if (ret!=SQLITE_DONE) {
        *errmsg += std::string("Error: ") + sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        return minids;
    }
    sqlite3_finalize(stmt);
    return minids;
}


/* Search the database for minerals that match the given criteria. Return a formated list of strings */
std::vector<std::string> db_search_minerals(sqlite3 *db, std::string sname, std::string sminid, std::string country, std::string species, 
        std::string orderby, std::string *errmsg) {

    std::string query = "SELECT ID, NAME FROM MINERALS ";
    query += "WHERE LOCALITY LIKE \"%" + country + "\" AND ";
    query += "( S1_SPECIES LIKE \"%"+species+"%\" or S2_SPECIES LIKE \"%"+species+"%\"";
    query += " or S3_SPECIES LIKE \"%"+species+"%\" or S4_SPECIES LIKE \"%"+species+"%\"";
    query += " or S1_VARIETY LIKE \"%"+species+"%\" or S2_VARIETY LIKE \"%"+species+"%\"";
    query += " or S3_VARIETY LIKE \"%"+species+"%\" or S4_VARIETY LIKE \"%"+species+"%\" ) ";
    query += "AND ( NAME LIKE \"%"+sname+"%\" OR ID LIKE \"%"+sminid+"%\" ) ";
    query += "ORDER BY " + orderby;

    std::vector<std::string> results;
    int ret;
    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    if (ret!=SQLITE_OK) {
        *errmsg += std::string(sqlite3_errmsg(db));
        return results;
    }
    const unsigned char *uc;
    std::string minid, name;
    while ((ret=sqlite3_step(stmt))==SQLITE_ROW) {
        uc = sqlite3_column_text(stmt, 0);
        if (uc!=NULL) minid=(const char*)uc; else minid="";
        uc = sqlite3_column_text(stmt, 1);
        if (uc!=NULL) name=(const char*)uc; else name="";
        results.push_back(name + " [" + minid + "]");
    }
    if (ret!=SQLITE_DONE) {
        *errmsg += std::string(sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return results;
}


/* Get a sorted list of files in the data directory for a given specimen */
std::vector<fs::path> db_get_datafile_list(std::string db_file_path, std::string minid) {
    std::vector<fs::path> files;
    /* Get basepath */
    fs::path basepath;
    basepath = fs::path(db_file_path).remove_filename() / "data";
    if (!fs::is_directory(basepath)) return files;
    basepath = fs::path(db_file_path).remove_filename() / "data" / minid;
    if (!fs::is_directory(basepath)) {
        std::string prefix = minid + " ";
        basepath = fs::path();
        for (const auto & entry : fs::directory_iterator(fs::path(db_file_path).remove_filename() / "data")) {
            if (entry.is_directory() && strncmp(entry.path().filename().c_str(), prefix.c_str(), prefix.size())==0) {
                basepath = entry.path();
                break;
            }
        }
        if (basepath.empty()) return files;
    }
    /* Get list of files */
    for (auto& entry: fs::directory_iterator(basepath)) {
        if (entry.path().filename().string().at(0) == '.') continue;
        files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    return files;
}

/* Get a formatted string for the size of a specimen */
std::string db_get_fmt_size(std::vector<std::string> data) {
    std::string l = db_get_field(data, "LENGTH");
    std::string w = db_get_field(data, "WIDTH");
    std::string h = db_get_field(data, "HEIGHT");
    std::string u = db_get_field(data, "SIZE_UNITS");
    std::string c = db_get_field(data, "SIZE_CATEGORY");
    std::string size;
    if (l.size()>0) size += l;
    if (w.size()>0) {
        if (size.size()>0) size += " x ";
        size += w;
    }
    if (h.size()>0) {
        if (size.size()>0) size += " x ";
        size += h;
    }
    if (u.size()>0) {
        if (size.size()>0) size += " ";
        size += u;
    }
    if (c.size()>0) {
        if (size.size()>0) size += " - ";
        size += c;
    }
    return size;
}

/* Get a formatted string for the weight of a specimen */
std::string db_get_fmt_weight(std::vector<std::string> data) {
    std::string w = db_get_field(data, "WEIGHT");
    std::string u = db_get_field(data, "WEIGHT_UNITS");
    std::string weight = w;
    if (weight.size()>0) weight += " ";
    weight += u;
    return weight;
}

/* Get a formatted string for the acquisition date & source */
std::string db_get_fmt_acquisition(std::vector<std::string> data) {
    std::string y = db_get_field(data, "ACQUISITION_YEAR");
    std::string m = db_get_field(data, "ACQUISITION_MONTH");
    std::string d = db_get_field(data, "ACQUISITION_DAY");
    std::string s = db_get_field(data, "ACQUISITION_SOURCE");
    std::string self = db_get_field(data, "SELFCOLLECTED");
    std::string a;
    std::vector<std::string> months = {"January", "February", "March", "April",
        "May", "June", "July", "August", "September", "October", "November", "December"};
    if (d.size()>0) a += d + " ";
    if (m.size()>0) {
        int mm, ret;
        ret = sscanf(m.c_str(), "%d", &mm);
        if (ret==1 and to_string(mm)==m) {
            if (mm>=0 and mm<12) {
                a += months[mm] + " ";
            } else {
                a += m + " ";
            }
        } else {
            a += m + " ";
        }
    }
    if (y.size()>0) a += y;
    if (s.size()>0) {
        if (a.size()>0) a += "; ";
        a += s;
    }
    if (self=="1") {
        if (a.size()>0) a += " ";
        a += "self-collected";
    }
    return a;
}

/* Get a formatted string for the deaccessioned date */
std::string db_get_fmt_deaccessioned(std::vector<std::string> data) {
    std::string y = db_get_field(data, "DEACCESSIONED_YEAR");
    std::string m = db_get_field(data, "DEACCESSIONED_MONTH");
    std::string d = db_get_field(data, "DEACCESSIONED_DAY");
    std::string s = db_get_field(data, "DEACCESSIONED_TO");
    std::string a;
    std::vector<std::string> months = {"January", "February", "March", "April",
        "May", "June", "July", "August", "September", "October", "November", "December"};
    if (d.size()>0) a += d + " ";
    if (m.size()>0) {
        int mm, ret;
        ret = sscanf(m.c_str(), "%d", &mm);
        if (ret==1 and to_string(mm)==m) {
            if (mm>=0 and mm<12) {
                a += months[mm] + " ";
            } else {
                a += m + " ";
            }
        } else {
            a += m + " ";
        }
    }
    if (y.size()>0) a += y;
    if (s.size()>0) {
        if (a.size()>0) a += "; ";
        a += s;
    }
    return a;
}


/* Get a formatted string for the value/price paid */
std::string db_get_fmt_value(std::vector<std::string> data, bool hidden) {
    std::string outstr;
    std::string value = db_get_field(data, "VALUE");
    std::string price = db_get_field(data, "PRICE");
    if (hidden) {
        outstr = "<hidden>";
    } else if (value.size()>0) {
        outstr += value;
        if (price.size()>0) {
            outstr += " (price paid: " + price + ")";
        }
    } else if (price.size()>0) {
        outstr = price;
    }
    return outstr;
}


/* Insert a new mineral or modify an existing one */
int db_addmod_mineral(sqlite3 *db, std::vector<std::string> data, int minid_mod, std::string *errmsg) {

    /* Check data vector size */
    size_t data_header_size = data_header.size();
    if (data.size()!=data_header_size) {
        *errmsg = std::string("Data should contain ") + std::to_string(data_header_size) + " items, but is has " + std::to_string(data.size());
        return -2;
    }

    /* Get the minid from data[0] */
    int ret;
    int minid_new;
    ret = sscanf(data[0].c_str(), "%d", &minid_new);
    if (ret!=1) minid_new = -1;
    data.erase(data.begin());

    /* Create SQL query */
    std::string query = "";
    if (minid_mod>0) {
        query += "UPDATE MINERALS SET ";
        for (size_t i=0; i<data_header_size-1; i++) { query += data_header[i] + "=?, "; }
        query += data_header[data_header_size-1] + "=? ";
        query += "WHERE ID=?;";
    } else {
        query += "INSERT ";
        if (minid_mod==-2) { query += "OR REPLACE "; }
        query += "INTO MINERALS (";
        if (minid_new>0) { query += "ID, "; }
        for (size_t i=1; i<data_header_size-1; i++) { query += data_header[i] + ", "; }
        query += data_header[data_header_size-1] + ") ";
        query += "VALUES (";
        for (size_t i=1; i<data_header_size-1; i++) { query += "?,"; }
        if (minid_new>0) { query += "?,"; }
        query += "?);";
    }

    /* Setup the db connection */
    sqlite3_stmt *stmt;
    int ndx = 1;
    sqlite3_prepare_v2(db, query.c_str(), strlen(query.c_str()), &stmt, NULL);
    if (minid_mod<0) {
        if (minid_new>0) {
            sqlite3_bind_int(stmt, ndx, minid_new); ndx+=1;
        }
    } else {
        if (minid_new<0) {
            minid_new = minid_mod;
        }
        sqlite3_bind_int(stmt, ndx, minid_new); ndx+=1;
    }

    /* Bind all inputs */
    for (size_t i=0; i<data_header_size-1; i++) {
        sqlite3_bind_text(stmt, ndx, data[i].c_str(), -1, SQLITE_TRANSIENT); ndx+=1;
    }
    if (minid_mod>=0) {
        sqlite3_bind_int(stmt, ndx, minid_mod); ndx+=1;
    }

    /* Run the query */
    ret = sqlite3_step(stmt);
    if (ret!=SQLITE_DONE) {
        *errmsg = std::string("Internal error: ") + std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -2;
    }

    sqlite3_finalize(stmt);
    return minid_new;
}

/* Get a list of unique countries in the db */
std::vector<std::string> db_get_country_list(sqlite3 *db, std::string *errmsg) {

    std::vector<std::string> loclst;
    const char *query = "SELECT LOCALITY FROM MINERALS";
    int ret;
    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (ret!=SQLITE_OK) {
        *errmsg += std::string(sqlite3_errmsg(db));
        return loclst;
    }
    std::string loc = "";
    std::string country = "";
    while ((ret=sqlite3_step(stmt))==SQLITE_ROW) {
        const unsigned char *uc = sqlite3_column_text(stmt, 0);
        loc = "";
        if (uc!=NULL) loc = (const char*)uc;
        if (loc.length()==0) continue;
        size_t pos = loc.rfind(", ");
        if (pos==std::string::npos) {
            country = loc;
        } else {
            country = loc.substr(pos+2, std::string::npos);
        }
        if (std::find(loclst.begin(), loclst.end(), country) == loclst.end()) {
            loclst.push_back(country);
        }
    }
    if (ret!=SQLITE_DONE) {
        *errmsg += std::string(sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    std::sort(loclst.begin(), loclst.end());
    return loclst;
}

/* Get a list of unique states in the db for the given country */
std::vector<std::string> db_get_state_list(sqlite3 *db, std::string country, std::string *errmsg) {

    std::vector<std::string> loclst;
    if (country=="") {
        return loclst;
    }

    std::string query = "SELECT LOCALITY FROM MINERALS WHERE LOCALITY LIKE \"%" + country + "\"";
    int ret;
    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    if (ret!=SQLITE_OK) {
        *errmsg += std::string(sqlite3_errmsg(db));
        return loclst;
    }
    std::string loc = "";
    std::string state = "";
    while ((ret=sqlite3_step(stmt))==SQLITE_ROW) {
        const unsigned char *uc = sqlite3_column_text(stmt, 0);
        loc = "";
        if (uc!=NULL) loc = (const char*)uc;
        if (loc.length()==0) continue;
        size_t pos = loc.rfind(", ");
        if (pos==std::string::npos) {
            state = "";
        } else {
            loc = loc.substr(0, pos);
            pos = loc.rfind(", ");
            if (pos==std::string::npos) {
                state = loc;
            } else {
                state = loc.substr(pos+2, std::string::npos);
            }
        }
        if (state!="" and std::find(loclst.begin(), loclst.end(), state) == loclst.end()) {
            loclst.push_back(state);
        }
    }
    if (ret!=SQLITE_DONE) {
        *errmsg += std::string(sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    std::sort(loclst.begin(), loclst.end());
    return loclst;
}

/* Get a list of unique species in the db */
std::vector<std::string> db_get_species_list(sqlite3 *db, std::string locality, std::string *errmsg) {

    std::vector<std::string> specieslst;
    std::string query = "SELECT S1_SPECIES,S2_SPECIES,S3_SPECIES,S4_SPECIES,S1_VARIETY,S2_VARIETY,S3_VARIETY,S4_VARIETY FROM MINERALS ";
    query += "WHERE LOCALITY LIKE \"%" + locality + "\"";
    int ret;
    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    if (ret!=SQLITE_OK) {
        *errmsg += std::string(sqlite3_errmsg(db));
        return specieslst;
    }
    std::string species = "";
    while ((ret=sqlite3_step(stmt))==SQLITE_ROW) {
        for (int i=0; i<8; i++) {
            const unsigned char *uc = sqlite3_column_text(stmt, i);
            species = "";
            if (uc!=NULL) species = (const char*)uc;
            if (species.length()==0) continue;
            if (std::find(specieslst.begin(), specieslst.end(), species) == specieslst.end()) {
                specieslst.push_back(species);
            }
        }
    }
    if (ret!=SQLITE_DONE) {
        *errmsg += std::string(sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    std::sort(specieslst.begin(), specieslst.end());
    return specieslst;
}


/* Format the chemical formula using html sub sup tags */
static std::string fmt_chemf(std::string chemf) {
    std::string outstr;
    strip_unicode(chemf);
    if (chemf=="") return outstr;
    bool subscript = true;
    if (chemf.find(" . ")!=std::string::npos) {
        chemf = chemf.replace(chemf.find(" . "), 3, "*");
    }
    for (size_t n=0; n<chemf.length(); n++) {
        char tchar = chemf[n];
        std::string tstr = std::string(1,tchar);
        char nchar = 0;
        if (n<chemf.length()-1) {
            nchar = chemf[n+1];
        }
        if (tchar=='+' || tchar=='-' || nchar=='+' || nchar=='-') {
            outstr += "<sup>" + tstr + "</sup>";
        } else if ((isdigit(tchar) || tchar=='.') && subscript) {
            outstr += "<sub>" + tstr + "</sub>";
        } else if (tchar=='*') {
            outstr += "\u00b7";
            //outstr += "<sup>.</sup>";
            subscript = false;
        } else {
            outstr += tstr;
            if (isalpha(tchar)) {
                subscript = true;
            }
        }
    }
    return outstr;
}


/* Generate an html-formatted report for one specimen */
static std::string db_generate_html_report_minid(std::vector<std::string> data) {

    std::string tmp;
    std::string html;

    html += "<h2 style=\"font-size:12pt\">" + db_get_field(data, "NAME") + "</h2>\n";
    html += "<table style=\"table-layout: fixed; width: 100%; font-size:10pt\">\n";
    html += "<colgroup>\n";
    html += "  <col style=\"width:20%\">\n";
    html += "  <col style=\"width:80%\">\n";
    html += "</colgroup>\n";
    html += "<tbody>\n";
    html += "  <tr><td><b>Number:</b></td><td>" + db_get_field(data, "ID") + "</td></tr>\n";
    html += "  <tr><td><b>MinID:</b></td><td><a href=\"https://www.mindat.org/" + db_get_field(data, "MINID") + "\">" + db_get_field(data, "MINID") + "</a></td></tr>\n";
    tmp = db_get_field(data, "LOCALITY");
    if (tmp.size()>0) {
        html += "  <tr><td><b>Locality:</b></td><td>" + tmp;
        tmp = db_get_field(data, "LOCALITY_ID");
        if (tmp.size()>0) {
            html += " <a href=\"https://www.mindat.org/loc-" + tmp + ".html\" target=\"_blank\">";
            html += "(mindat: " + tmp +")";
        }
        html += "</td></tr>\n";
    }
    tmp = html_escape(db_get_fmt_size(data));
    if (tmp.size()>0) html += "  <tr><td><b>Size:</b></td><td>" + tmp + "</td></tr>\n";
    tmp = db_get_fmt_weight(data);
    if (tmp.size()>0) html += "  <tr><td><b>Weight:</b></td><td>" + tmp + "</td></tr>\n";
    tmp = db_get_fmt_acquisition(data);
    if (tmp.size()>0) html += "  <tr><td><b>Acquisition:</b></td><td>" + tmp + "</td></tr>\n";
    tmp = db_get_fmt_deaccessioned(data);
    if (tmp.size()>0) html += "  <tr><td><b>Deaccessioned:</b></td><td>" + tmp + "</td></tr>\n";
    tmp = db_get_field(data, "COLLECTION");
    if (tmp.size()>0) html += "  <tr><td><b>Collection:</b></td><td>" + tmp + "</td></tr>\n";
    tmp = db_get_fmt_value(data, false);
    if (tmp.size()>0) html += "  <tr><td><b>Estimated value:</b></td><td>" + tmp + "</td></tr>\n";
    html += "</tbody>\n";
    html += "</table>\n";

    /* Species */
    html += "<table class=\"species\" style=\"table-layout: fixed; width: 100%; font-size:10pt\">\n";
    html += "<colgroup>\n";
    html += "  <col style=\"width:20%\">\n";
    html += "  <col style=\"width:20%\">\n";
    html += "  <col style=\"width:20%\">\n";
    html += "  <col style=\"width:20%\">\n";
    html += "  <col style=\"width:20%\">\n";
    html += "</colgroup>\n";
    html += "<tbody>\n";
    tmp = db_get_field(data, "S1_SPECIES") + db_get_field(data, "S2_SPECIES") + db_get_field(data, "S3_SPECIES") + db_get_field(data, "S4_SPECIES");
    if (tmp.size()>0) {
        html += "<tr><td><b>Species:</b></td><td>" + db_get_field(data, "S1_SPECIES") + "</td><td>" + db_get_field(data, "S2_SPECIES") + "</td><td>" + db_get_field(data, "S3_SPECIES") + "</td><td>" + db_get_field(data, "S4_SPECIES") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_VARIETY") + db_get_field(data, "S2_VARIETY") + db_get_field(data, "S3_VARIETY") + db_get_field(data, "S4_VARIETY");
    if (tmp.size()>0) {
        html += "<tr><td><b>Variety:</b></td><td>" + db_get_field(data, "S1_VARIETY") + "</td><td>" + db_get_field(data, "S2_VARIETY") + "</td><td>" + db_get_field(data, "S3_VARIETY") + "</td><td>" + db_get_field(data, "S4_VARIETY") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_CLASS") + db_get_field(data, "S2_CLASS") + db_get_field(data, "S3_CLASS") + db_get_field(data, "S4_CLASS");
    if (tmp.size()>0) {
        html += "<tr><td><b>Class:</b></td><td>" + db_get_field(data, "S1_CLASS") + "</td><td>" + db_get_field(data, "S2_CLASS") + "</td><td>" + db_get_field(data, "S3_CLASS") + "</td><td>" + db_get_field(data, "S4_CLASS") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_CHEMF") + db_get_field(data, "S2_CHEMF") + db_get_field(data, "S3_CHEMF") + db_get_field(data, "S4_CHEMF");
    if (tmp.size()>0) {
        html += "<tr><td><b>Chemical Formula:</b></td><td>" + fmt_chemf(db_get_field(data, "S1_CHEMF")) + "</td><td>" + fmt_chemf(db_get_field(data, "S2_CHEMF")) + "</td><td>" + fmt_chemf(db_get_field(data, "S3_CHEMF")) + "</td><td>" + fmt_chemf(db_get_field(data, "S4_CHEMF")) + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_COLOR") + db_get_field(data, "S2_COLOR") + db_get_field(data, "S3_COLOR") + db_get_field(data, "S4_COLOR");
    if (tmp.size()>0) {
        html += "<tr><td><b>Color:</b></td><td>" + db_get_field(data, "S1_COLOR") + "</td><td>" + db_get_field(data, "S2_COLOR") + "</td><td>" + db_get_field(data, "S3_COLOR") + "</td><td>" + db_get_field(data, "S4_COLOR") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_TRANSP") + db_get_field(data, "S2_TRANSP") + db_get_field(data, "S3_TRANSP") + db_get_field(data, "S4_TRANSP");
    if (tmp.size()>0) {
        html += "<tr><td><b>Transparency:</b></td><td>" + db_get_field(data, "S1_TRANSP") + "</td><td>" + db_get_field(data, "S2_TRANSP") + "</td><td>" + db_get_field(data, "S3_TRANSP") + "</td><td>" + db_get_field(data, "S4_TRANSP") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_HABIT") + db_get_field(data, "S2_HABIT") + db_get_field(data, "S3_HABIT") + db_get_field(data, "S4_HABIT");
    if (tmp.size()>0) {
        html += "<tr><td><b>Habit:</b></td><td>" + db_get_field(data, "S1_HABIT") + "</td><td>" + db_get_field(data, "S2_HABIT") + "</td><td>" + db_get_field(data, "S3_HABIT") + "</td><td>" + db_get_field(data, "S4_HABIT") + "</td></tr>\n";
    }

    /* Fluorescence */
    tmp = db_get_field(data, "S1_FLSW") + db_get_field(data, "S2_FLSW") + db_get_field(data, "S3_FLSW") + db_get_field(data, "S4_FLSW");
    if (tmp.size()>0) {
        html += "<tr><td><b>Fluorescence SW:</b></td><td>" + db_get_field(data, "S1_FLSW") + "</td><td>" + db_get_field(data, "S2_FLSW") + "</td><td>" + db_get_field(data, "S3_FLSW") + "</td><td>" + db_get_field(data, "S4_FLSW") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_FLMW") + db_get_field(data, "S2_FLMW") + db_get_field(data, "S3_FLMW") + db_get_field(data, "S4_FLMW");
    if (tmp.size()>0) {
        html += "<tr><td><b>Fluorescence MW:</b></td><td>" + db_get_field(data, "S1_FLMW") + "</td><td>" + db_get_field(data, "S2_FLMW") + "</td><td>" + db_get_field(data, "S3_FLMW") + "</td><td>" + db_get_field(data, "S4_FLMW") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_FLLW") + db_get_field(data, "S2_FLLW") + db_get_field(data, "S3_FLLW") + db_get_field(data, "S4_FLLW");
    if (tmp.size()>0) {
        html += "<tr><td><b>Fluorescence LW:</b></td><td>" + db_get_field(data, "S1_FLLW") + "</td><td>" + db_get_field(data, "S2_FLLW") + "</td><td>" + db_get_field(data, "S3_FLLW") + "</td><td>" + db_get_field(data, "S4_FLLW") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_FL405") + db_get_field(data, "S2_FL405") + db_get_field(data, "S3_FL405") + db_get_field(data, "S4_FL405");
    if (tmp.size()>0) {
        html += "<tr><td><b>Fluorescence 405nm:</b></td><td>" + db_get_field(data, "S1_FL405") + "</td><td>" + db_get_field(data, "S2_FL405") + "</td><td>" + db_get_field(data, "S3_FL405") + "</td><td>" + db_get_field(data, "S4_FL405") + "</td></tr>\n";
    }

    /* Phosphorescence */
    tmp = db_get_field(data, "S1_PHSW") + db_get_field(data, "S2_PHSW") + db_get_field(data, "S3_PHSW") + db_get_field(data, "S4_PHSW");
    if (tmp.size()>0) {
        html += "<tr><td><b>Phosphorescence SW:</b></td><td>" + db_get_field(data, "S1_PHSW") + "</td><td>" + db_get_field(data, "S2_PHSW") + "</td><td>" + db_get_field(data, "S3_PHSW") + "</td><td>" + db_get_field(data, "S4_PHSW") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_PHMW") + db_get_field(data, "S2_PHMW") + db_get_field(data, "S3_PHMW") + db_get_field(data, "S4_PHMW");
    if (tmp.size()>0) {
        html += "<tr><td><b>Phosphorescence MW:</b></td><td>" + db_get_field(data, "S1_PHMW") + "</td><td>" + db_get_field(data, "S2_PHMW") + "</td><td>" + db_get_field(data, "S3_PHMW") + "</td><td>" + db_get_field(data, "S4_PHMW") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_PHLW") + db_get_field(data, "S2_PHLW") + db_get_field(data, "S3_PHLW") + db_get_field(data, "S4_PHLW");
    if (tmp.size()>0) {
        html += "<tr><td><b>Phosphorescence LW:</b></td><td>" + db_get_field(data, "S1_PHLW") + "</td><td>" + db_get_field(data, "S2_PHLW") + "</td><td>" + db_get_field(data, "S3_PHLW") + "</td><td>" + db_get_field(data, "S4_PHLW") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_PH405") + db_get_field(data, "S2_PH405") + db_get_field(data, "S3_PH405") + db_get_field(data, "S4_PH405");
    if (tmp.size()>0) {
        html += "<tr><td><b>Phosphorescence 405nm:</b></td><td>" + db_get_field(data, "S1_PH405") + "</td><td>" + db_get_field(data, "S2_PH405") + "</td><td>" + db_get_field(data, "S3_PH405") + "</td><td>" + db_get_field(data, "S4_PH405") + "</td></tr>\n";
    }

    /* Other properties */
    tmp = db_get_field(data, "S1_TENEBR") + db_get_field(data, "S2_TENEBR") + db_get_field(data, "S3_TENEBR") + db_get_field(data, "S4_TENEBR");
    if (tmp.size()>0) {
        html += "<tr><td><b>Tenebrescence:</b></td><td>" + db_get_field(data, "S1_TENEBR") + "</td><td>" + db_get_field(data, "S2_TENEBR") + "</td><td>" + db_get_field(data, "S3_TENEBR") + "</td><td>" + db_get_field(data, "S4_TENEBR") + "</td></tr>\n";
    }
    tmp = db_get_field(data, "S1_TRIBO") + db_get_field(data, "S2_TRIBO") + db_get_field(data, "S3_TRIBO") + db_get_field(data, "S4_TRIBO");
    if (tmp.size()>0) {
        html += "<tr><td><b>Triboluminescence:</b></td><td>" + db_get_field(data, "S1_TRIBO") + "</td><td>" + db_get_field(data, "S2_TRIBO") + "</td><td>" + db_get_field(data, "S3_TRIBO") + "</td><td>" + db_get_field(data, "S4_TRIBO") + "</td></tr>\n";
    }

    /* Radioactivity and Comments */
    tmp = db_get_field(data, "RADIOACTIVITY");
    if (tmp.size()>0) {
        html += "<tr><td><b>Radioactive:</b></td><td colspan=\"4\">" + tmp + "</td></tr>\n";
    }
    html += "</tbody>\n";
    html += "</table>\n";

    tmp = html_escape(db_get_field(data, "DESCRIPTION"));
    if (tmp.size()>0) html += "<p style=\"font-size:10pt\"><b>Description:</b><br>" + tmp + "<br></p>\n";
    tmp = html_escape(db_get_field(data, "NOTES"));
    if (tmp.size()>0) html += "<p style=\"font-size:10pt\"><b>Notes:</b><br>" + tmp + "<br></p>\n";
    tmp = html_escape(db_get_field(data, "OWNERS"));
    if (tmp.size()>0) html += "<p style=\"font-size:10pt\"><b>Previous owners:</b><br>" + tmp + "<br></p>\n";
    return html;
}

/* Generate html report for the images */
static std::string db_generate_html_report_minid_images(std::string db_file_path, std::string minid) {
    std::vector<fs::path> files = db_get_datafile_list(db_file_path, minid);
    std::vector<std::string> img_formats = { ".png", ".jpg", ".jpeg", ".gif", ".tiff", ".tif" };
    std::vector<fs::path> images;
    for (const auto & path : files) {
        std::string ext = path.extension().string();
        if (std::find(img_formats.begin(), img_formats.end(), ext) != img_formats.end()) {
            images.push_back(path);
        }
    }
    if (images.size()==0) return "";
    std::string html;
    html += "<br>\n";
    for (const auto & path : images) {
        html += "<img src=\"" + path.string() + "\"><br>\n";
    }
    html += "<br>\n";
    return html;
}


/* Generate an html-formatted report for the whole database */
static std::string db_generate_html_report(sqlite3 *db, std::vector<int> minids, std::string db_file_path, bool incdata, std::string *errmsg) {
    std::vector<std::string> data;
    std::string html;
    html += "<html xmlns:o='urn:schemas-microsoft-com:office:office' xmlns:w='urn:schemas-microsoft-com:office:word' xmlns='http://www.w3.org/TR/REC-html40'>\n";
    html += "<head><title>Mineral Database -- generated by MineralApp</title>\n";
    html += "<style>\n";
    html += "@page WordSection1 {size:5.5in 8.5in;margin:0.5in 0.5in 0.5in 0.5in;mso-header-margin:.5in;mso-footer-margin:.5in;mso-paper-source:0;}";
    html += "div.WordSection1 {page:WordSection1;}\n";
    html += "table.species, table.species td {\n border: 1px solid black;\n  border-collapse: collapse;\n}";
    html += "</style>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "<div class=WordSection1>\n";
    for (const auto& minid: minids) {
        data = db_get_data(db, minid, errmsg);
        if (errmsg->size()>0) return std::string();
        html += db_generate_html_report_minid(data);
        if (incdata) html += db_generate_html_report_minid_images(db_file_path, std::to_string(minid));
        html += "<br clear=all style='mso-special-character:line-break;page-break-before:always'>\n";
    }
    html += "</div>\n";
    html += "</body>\n";
    html += "</html>\n";
    return html;
}

/* Generate report -- main function */
void db_generate_report(sqlite3* db, std::string db_file_path, std::string fname, bool fulldb, bool incdata, int selected_uid, std::string *errmsg) {

    /* Prepare list of minids */
    std::vector<int> minids;
    if (fulldb) {
        minids = db_get_minid_list(db, 0, errmsg);
        if (errmsg->size()>0) return;
    } else if (selected_uid>=0) {
        minids.push_back(selected_uid);
    } else {
        *errmsg += "Sorry, no mineral was selected.";
        return;
    }

    /* Generate html report */
    std::string htmlreport = db_generate_html_report(db, minids, db_file_path, incdata, errmsg);
    if (errmsg->size()>0) return;

    /* Save html */
    std::ofstream fp(fname);
    fp << htmlreport;
    fp.close();

    return;
}



/*****************************************************************************
    CSV Import/Export
*****************************************************************************/

/* CSV import -- Utility function: Check that the header file is correct */
static bool db_csv_import_check(std::string filename, std::string *errmsg) {
    std::ifstream f(filename);
    aria::csv::CsvParser parser(f);
    int index = 0;
    for (;;) {
        auto field = parser.next_field();
        switch (field.type) {
            case aria::csv::FieldType::DATA:
                if (*field.data==data_header[index]) {
                    index++;
                } else {
                    *errmsg = std::string("Column ") + std::to_string(index+1) + " should be " + data_header[index] + " but I got " + *field.data;
                    return false;
                }
                break;
            case aria::csv::FieldType::ROW_END:
                return true;
            case aria::csv::FieldType::CSV_END:
                return false;
        }
    }
    return false;
}

/* CSV import -- Utility function: Read all data from a CSV file */
static bool db_csv_import_core(sqlite3 *db, std::string filename, std::string *errmsg, bool skip_miss_id) {
    int success_id;
    int ret;
    int minid;
    std::ifstream f(filename);
    aria::csv::CsvParser parser(f);
    std::vector<std::string> data;
    int rowndx = 0;
    for (auto& row : parser) {
        rowndx++;
        data.clear();
        for (auto& field : row) {
            data.push_back(field);
        }
        if (rowndx>1) {
            ret = sscanf(data[0].c_str(), "%d", &minid);
            if (ret!=1) minid = -1;
            if (skip_miss_id) {
                if (minid<0) continue;
            } else {
                if (minid>0) continue;
            }
            success_id = db_addmod_mineral(db, data, -2, errmsg);
            if (success_id<-1) {
                *errmsg = *errmsg + " While reading minid " + data[0] + " " + data[1];
                return false;
            }
        }
    }
    return true;
}

/* CSV import -- Main driver */
bool db_csv_import(sqlite3 *db, std::string filename, std::string *errmsg) {
    if (!db_csv_import_check(filename, errmsg)) return false;
    if (!db_csv_import_core(db, filename, errmsg, true)) return false;
    if (!db_csv_import_core(db, filename, errmsg, false)) return false;
    return true;
}

/* CSV export */
bool db_csv_export(sqlite3 *db, std::string filename, std::string *errmsg) {

    std::ofstream csvfile;
    csvfile.open(filename);
    size_t data_header_size = data_header.size();

    /* Write CSV header */
    for (size_t i=0; i<data_header_size-1; i++) {
        csvfile << data_header[i] << ",";
    }
    csvfile << data_header[data_header_size-1] << std::endl;

    /* Loop over all minid and write data */
    std::vector<int> minids = db_get_minid_list(db, 0, errmsg);
    for (auto minid : minids) {
        std::vector<std::string> data = db_get_data(db, minid, errmsg);
        for (size_t i=0; i<data_header_size; i++) {
            std::string s = db_get_field(data, data_header[i], false);
            csvfile << "\"" << str_escape(s, '"', '"') << "\"";
            if (i+1!=data_header_size) csvfile << ",";
        }
        csvfile << std::endl;
    }

    csvfile.close();
    return true;
}


/*****************************************************************************
    General utilities
*****************************************************************************/

/* Convert a string to a valid URL */
std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            escaped << c;
            continue;
        }
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }
    return escaped.str();
}

/* Escape all occurences of to_escape with escape_with. */
std::string str_escape(const std::string &value, char to_escape, char escape_with) {
    std::ostringstream escaped;
    escaped.fill('0');
    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        if (c==to_escape) escaped << escape_with;
        escaped << c;
    }
    return escaped.str();
}

/* Convert a string to lowercase. */
std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

/* Remove all unicode symbols from a string. */
static bool invalid_char(char c) {
    return c<0;
}
std::string strip_unicode(std::string &str) {
    str.erase(remove_if(str.begin(),str.end(), invalid_char), str.end());
    return str;
}

/* Escape char for a valid HTML output */
std::string html_escape(std::string s) {
    std::ostringstream e;
    e.fill('0');
    for (std::string::const_iterator i = s.begin(), n = s.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        if (c=='<') e << "&lt;";
        else if (c=='>') e << "&gt;";
        else if (c=='\n') e << "<br>";
        else e << c;
    }
    return e.str();
}

