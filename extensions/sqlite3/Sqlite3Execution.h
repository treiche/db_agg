/*
 * Sqlite3Execution3.h
 *
 *  Created on: Jun 18, 2014
 *      Author: arnd
 */

#ifndef SQLITE3EXECUTION_H_
#define SQLITE3EXECUTION_H_

#include "table/TableData.h"
#include <memory>

extern "C" {
#include <sqlite3.h>
}


namespace db_agg {
class Sqlite3Execution: public db_agg::QueryExecution {
private:
    uint64_t lastOffset = 0;
    uint64_t chunkSize = 1000;
    std::shared_ptr<db_agg::TableData> resultTable;
    sqlite3_stmt *stmt;
    sqlite3 *db;
    bool tableCreated = false;
public:
    virtual bool process() override;
    virtual bool isResourceAvailable() override;
};
}




#endif /* SQLITE3EXECUTION_H_ */
