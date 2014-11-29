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

namespace db_agg {
class Sqlite3Execution: public db_agg::QueryExecution {
private:
    uint64_t lastOffset = 0;
    uint64_t chunkSize = 1000;
    std::shared_ptr<db_agg::TableData> resultTable;
    uint32_t keyColIdx = 0;
public:
    virtual bool process() override;
};
}




#endif /* SQLITE3EXECUTION_H_ */
