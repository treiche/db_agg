/*
 * PGQueryExecution.h
 *
 *  Created on: Jun 10, 2014
 *      Author: arnd
 */

#ifndef PGQUERYEXECUTION_H_
#define PGQUERYEXECUTION_H_

#include <vector>
#include <map>
#include "ExecutionHandler.h"
#include "core/QueryExecution.h"
#include "AsyncQueryExecutor.h"

namespace db_agg {

class PGQueryExecution: public ExecutionHandler, public QueryExecution, public EventListener {
private:
    AsyncQueryExecutor queryExecutor;
    std::string toPostgresUrl(std::shared_ptr<Url> url);
    std::shared_ptr<TableData> resultTable;
public:
    // ExecutionHandler
    PGQueryExecution();
    virtual void handleCopyIn(size_t step, uint64_t startRows, uint64_t rows, std::vector<DataChunk>& chunks, uint64_t& rowsRead) override;
    virtual uint64_t getRowCount(size_t step) override;
    virtual void handleCopyOut(size_t step, std::string data) override;
    virtual void handleTuples(size_t step, std::vector<std::pair<std::string, uint32_t>>& columns) override;
    // QueryExecution
    virtual void stop() override;
    virtual void schedule() override;
    virtual bool process() override;
    virtual void cleanUp() override;
    virtual bool isResourceAvailable() override;
    // EventListener
    virtual void handleEvent(std::shared_ptr<Event> event) override;
};

}

#endif /* PGQUERYEXECUTION_H_ */
