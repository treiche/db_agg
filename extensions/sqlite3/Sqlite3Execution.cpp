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


using namespace std;
using namespace db_agg;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("Sqlite3Execution");

/*
 static int callback(void*, int, char**, char**) {
 cout << "called callback " << endl;
 return 0;
 }
 */

static int xEntryPoint(sqlite3 *db, const char **pzErrMsg,
        const struct sqlite3_api_routines *pThunk) {
    return sq3_register_virtual_tabledata(db);
}

bool Sqlite3Execution::isResourceAvailable() {
    string path = getUrl()->getPath();
    LOG_DEBUG("check if " << path << " exists");
    File dbFile(path);
    return dbFile.exists();
}

bool Sqlite3Execution::process() {
    char *zErrMsg = 0;
    int rc;
    LOG_DEBUG("process " << lastOffset);

    if (lastOffset == 0) {

        LOG_DEBUG("process " << getUrl()->getUrl());
        if (getUrl()->getProtocol() != "file") {
            THROW_EXC("only protocol 'file' supported yet");
        }

        for (auto& dep : getDependencies()) {
            LOG_DEBUG("register dependency " << dep.first);
            registerTableData(dep.first, dep.second);
        }

        sqlite3_auto_extension((void (*)(void))xEntryPoint);
        string dbFile = "/" + getUrl()->getPath();
        LOG_DEBUG("open db " << dbFile);
        rc = sqlite3_open_v2(dbFile.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
        LOG_DEBUG("process db = " << db);
        if (rc) {
            THROW_EXC("open database failed")
        }
        sqlite3_enable_load_extension(db, 0);
        //string sql = getSql() + "; create virtual table tbl using csvfile(/home/arnd/scratchpad/db_agg_github/tmp/csvfile/test.csv);";

        for (auto& dep : getDependencies()) {
            string sql = "create virtual table if not exists " + dep.first
                    + " using dbagg(" + dep.first + ");";

            LOG_DEBUG("execute '" << sql << "'")
            rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
            if (rc != SQLITE_OK) {
                THROW_EXC("err = " << zErrMsg);
            }
            shared_ptr<Event> evs(new SentDataEvent(getId(),dep.second->getRowCount()));
            fireEvent(evs);
        }

        string sql = getSql();
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            THROW_EXC("preparation of '" << sql << "' failed:" << sqlite3_errmsg(db));
        }
        shared_ptr<Event> ev(new ExecutionStateChangeEvent(getId(),"CONNECTED"));
        fireEvent(ev);

        vector<string> colHeads;
        int n = sqlite3_column_count(stmt);
        for (int i = 0; i < n; i++) {
            string colName(sqlite3_column_name(stmt, i));
            //string colType(sqlite3_column_decltype(stmt, i));
            const char *ct = sqlite3_column_decltype(stmt, i);
            if (!ct) {
                ct = "text";
            }
            string head = colName + ":" + ct;
            colHeads.push_back(head);
        }
        resultTable = TableDataFactory::getInstance().create("text",
                colHeads);

    }

    uint64_t row;
    int n;
    bool queryDone = false;
    for (row = lastOffset; row < lastOffset + chunkSize; row++) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            queryDone = true;
            break;
        }
        vector<string> colVals;
        switch (rc) {
        case SQLITE_ERROR:
            throw runtime_error("SQLITE_ERROR");
            break;
        case SQLITE_MISUSE:
            throw runtime_error("SQLITE_MISUSE");
            break;
        case SQLITE_ROW:
            /*
            if (!tableCreated) {
                for (int i = 0; i < n; i++) {
                    string colName(sqlite3_column_name(stmt, i));
                    colHeads.push_back(make_pair(colName, TEXT));
                }
                resultTable = TableDataFactory::getInstance().create("text",
                        colHeads);
                tableCreated = true;
            }
            */
            n = sqlite3_column_count(stmt);
            colVals.clear();
            for (int i = 0; i < n; i++) {
                char *val = (char *) sqlite3_column_text(stmt, i);
                string colName(sqlite3_column_name(stmt, i));
                if (val) {
                    colVals.push_back(string(val));
                } else {
                    colVals.push_back("\\N");
                }
            }
            resultTable->addRow(colVals);
            break;
        default:
            THROW_EXC("unknown error code " << rc);
        }
    }


    shared_ptr<Event> rde(new ReceiveDataEvent(getId(),resultTable->getRowCount()));
    fireEvent(rde);

    if (queryDone) {
        LOG_DEBUG("drop virtual tables " << getDependencies().size());
        for (auto& dep : getDependencies()) {
            string sql = "drop table " + dep.first + ";";
            LOG_DEBUG(sql);
            zErrMsg = nullptr;
            rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
            if (rc != SQLITE_OK) {
                LOG_DEBUG(
                        "err = code = " << rc << zErrMsg << " code = " << rc << " code");
            }
        }
        sqlite3_close(db);

        setResult("", resultTable);
        shared_ptr<Event> event(new Event(EventType::PROCESSED, getId()));
        fireEvent(event);
        shared_ptr<Event> e(new ExecutionStateChangeEvent(getId(), "DONE"));
        EventProducer::fireEvent(e);
        setDone();
        LOG_DEBUG("process done");
        return true;
    }
    lastOffset = row;
    return false;
}

}

