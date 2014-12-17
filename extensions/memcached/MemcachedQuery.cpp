/*
 * Memcached.cpp
 *
 *  Created on: Jun 18, 2014
 *      Author: arnd
 */

#include <log4cplus/logger.h>


#include "db_agg.h"
#include "MemcachedQuery.h"
#include "Memcached.h"

extern "C" {
#include <jansson.h>
}


using namespace std;
using namespace db_agg;
using namespace log4cplus;


namespace memcached {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("MemcachedQuery"));


bool MemcachedQuery::process() {
    assert(getDependencies().size() == 1);
    assert(getArguments().size() == 1);
    LOG_DEBUG("process memcached query: " << getUrl()->getUrl() << " lastOffset " << lastOffset << " chunkSize = " << chunkSize);

    // initialization
    if (lastOffset == 0) {
        map<string,shared_ptr<TableData>> deps = getDependencies();
        this->sourceTable = (*deps.begin()).second;
        this->keyColIdx = sourceTable->getColumnIndex(getArguments()[0]);
        vector<ColDef> resultCols;
        resultCols.push_back(ColDef{"mc_result",TEXT});
        this->memcachedResultTable = TableDataFactory::getInstance().create("text",resultCols);
        shared_ptr<Event> ev(new ExecutionStateChangeEvent(getId(),"CONNECTED"));
        fireEvent(ev);
    }

    // process next chunk
    vector<string> keys;
    for (uint64_t row = lastOffset; row < sourceTable->getRowCount() && row < lastOffset + chunkSize; row++) {
        LOG_DEBUG("get value for row " << row);
        keys.push_back(sourceTable->getValue(row,keyColIdx));
    }

    string server = getUrl()->getHost() + ":" + getUrl()->getPort();
    Memcached mc({server});
    vector<string> resultValues = mc.getMulti(keys);
    uint64_t row;
    for (row = lastOffset; row < sourceTable->getRowCount() && row < lastOffset + chunkSize; row++) {
        vector<string> r;
        r.push_back(resultValues[row-lastOffset]);
        memcachedResultTable->addRow(r);
    }

    shared_ptr<Event> event(new ReceiveDataEvent(getId(),row));
    fireEvent(event);

    if (row >= sourceTable->getRowCount() -1) {
        auto resultTable = TableDataFactory::getInstance().extend({sourceTable,memcachedResultTable});
        LOG_DEBUG("result colCount = " << resultTable->getColCount());

        setResult("", resultTable);
        setState(QueryExecutionState::DONE);
        shared_ptr<Event> event(new Event(EventType::PROCESSED,getId()));
        fireEvent(event);
        shared_ptr<Event> e(new ExecutionStateChangeEvent(getId(),"DONE"));
        EventProducer::fireEvent(e);
        LOG4CPLUS_DEBUG(LOG,"process done");
        return true;
    } else {
        lastOffset = row;
        return false;
    }
}

}



