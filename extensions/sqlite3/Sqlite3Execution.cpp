/*
 * Sqlite3Execution.cpp
 *
 *  Created on: Jun 18, 2014
 *      Author: arnd
 */

#include <log4cplus/logger.h>

#include "db_agg.h"
#include "Sqlite3Execution.h"
#include "VirtualTableData.h"

extern "C" {
#include <sqlite3.h>
}

using namespace std;
using namespace db_agg;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("Sqlite3Execution");

static int callback(void*, int, char**, char**) {
    cout << "called callback " << endl;
    return 0;
}

static int xEntryPoint(sqlite3 *db, const char **pzErrMsg,
        const struct sqlite3_api_routines *pThunk) {
    cout << "called entry point for db " << db << " pThunk = " << pThunk
            << endl;
    int rc = sq3_register_virtual_tabledata(db);
    cout << "register virtual tabledata returned " << rc << endl;
    return rc;
}

bool Sqlite3Execution::process() {
    LOG_ERROR("query " << getSql());

    registerTableData("sqdep",
            TableDataFactory::getInstance().load(
                    "/home/arnd/scratchpad/db_agg_github/tests/data/external2.csv"));

    sqlite3_auto_extension((void (*)(void))xEntryPoint);sqlite3
    *db;
    sqlite3_stmt *stmt;
    int rc;
    char *zErrMsg = 0;
    rc = sqlite3_open_v2("test.db", &db,
    SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
    cout << "process db = " << db << endl;
    if (rc) {
        THROW_EXC("open database failed")
    }
    sqlite3_enable_load_extension(db, 0);
    //string sql = getSql() + "; create virtual table tbl using csvfile(/home/arnd/scratchpad/db_agg_github/tmp/csvfile/test.csv);";

    rc = sqlite3_exec(db,
            "create virtual table if not exists sqdep using dbagg(sqdep);",
            callback, nullptr, &zErrMsg);
    if (rc == 1) {
        cout << "err = " << zErrMsg << endl;
    }
    // rc = sqlite3_exec(db,"select 1;",callback,nullptr,&zErrMsg);
    cout << "exec returned " << rc << endl;

    string sql = getSql();
    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        THROW_EXC("prepared failed");
    }
    bool tableCreated = false;
    while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        cout << "rc = " << rc << endl;
        int n;
        vector<pair<string, uint32_t>> colHeads;
        vector<string> colVals;
        switch (rc) {
        case SQLITE_ERROR:
            throw runtime_error("SQLITE_ERROR");
            break;
        case SQLITE_MISUSE:
            throw runtime_error("SQLITE_MISUSE");
            break;
        case SQLITE_ROW:
            n = sqlite3_column_count(stmt);
            if (!tableCreated) {
                for (int i = 0; i < n; i++) {
                    string colName(sqlite3_column_name(stmt, i));
                    cout << "add col " << colName << endl;
                    colHeads.push_back(make_pair(colName, TEXT));
                }
                resultTable = TableDataFactory::getInstance().create("text",
                        colHeads);
                tableCreated = true;
            }
            colVals.clear();
            for (int i = 0; i < n; i++) {
                cout << sqlite3_column_text(stmt, i) << endl;
                char *val = (char *) sqlite3_column_text(stmt, i);
                if (val) {
                    colVals.push_back(string(val));
                } else {
                    colVals.push_back("\\N");
                }
            }
            resultTable->addRow(colVals);
            break;
        default:
            THROW_EXC("unknown error code " << rc)
            ;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    setResult("", resultTable);
    shared_ptr<Event> event(new Event(EventType::PROCESSED, getId()));
    fireEvent(event);
    shared_ptr<Event> e(new ExecutionStateChangeEvent(getId(), "DONE"));
    EventProducer::fireEvent(e);
    setDone();
    LOG_ERROR("process done");

    rc = sqlite3_exec(db, "drop table sqdep;", callback, nullptr, &zErrMsg);
    if (rc == 1) {
        cout << "err = " << zErrMsg << endl;
    }
    return true;
}

}

