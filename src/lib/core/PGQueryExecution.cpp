/*
 * PGQueryExecution.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: arnd
 */

#include "PGQueryExecution.h"

#include "utils/logging.h"

#include "type/oids.h"
#include "table/TableDataFactory.h"
#include "postgres/PGConnection.h"


using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("QueryExecution"));

    PGQueryExecution::PGQueryExecution(): QueryExecution() {
        queryExecutor.addEventListener(this);
    }

    uint64_t PGQueryExecution::getRowCount(size_t stepNo) {
        ExecutionStep& step = getInjector()->getStep(stepNo);
        return step.getDependency()->getRowCount();
    }

    void PGQueryExecution::handleCopyIn(size_t stepNo, uint64_t startRow, uint64_t rows, vector<DataChunk>& chunks, uint64_t& rowsRead) {
        ExecutionStep& step = getInjector()->getStep(stepNo);
        uint64_t rowCount = step.getDependency()->getRowCount();
        if (startRow + rows > rowCount) {
            rowsRead = rowCount - startRow;
        } else {
            rowsRead = rows;
        }
        step.getDependency()->getRows(startRow, rowsRead, chunks);
    }

    void PGQueryExecution::handleCopyOut(size_t stepNo,string _data) {
         getData()->appendRaw((void*)_data.c_str(),_data.size());
    }

    void PGQueryExecution::handleTuples(size_t step, vector<pair<string, uint32_t> >& columns) {
        vector<pair<string, uint32_t>> cleaned;
        for (size_t idx = 0; idx < columns.size(); idx++) {
            auto column = columns[idx];
            if (column.first == "?column?") {
                LOG_DEBUG("column " << idx << "has unknown name/type. assume type TEXT");
                cleaned.push_back(pair<string,uint32_t>(string("argument_") + to_string(idx),TEXT));
            } else {
                cleaned.push_back(column);
            }
        }
        setData(TableDataFactory::getInstance().create("text",cleaned));
    }

    void PGQueryExecution::stop() {
        queryExecutor.stop();
    }

    void PGQueryExecution::schedule() {
        string injectedSql = this->inject(getSql(), 0);
        queryExecutor.addQuery(getId(),toPostgresUrl(getUrl()), injectedSql, this);
        setScheduled();
    }

    bool PGQueryExecution::process() {
        LOG_DEBUG("process postgres query '" << getName() << "'");
        return queryExecutor.process();
    }

    // EventListener
    void PGQueryExecution::handleEvent(Event& event) {
        fireEvent(event);
    }

    void PGQueryExecution::cleanUp() {
        queryExecutor.cleanUp("CANCEL");
    }

    string PGQueryExecution::toPostgresUrl(shared_ptr<Url> url) {
        string pgurl = "host=" + url->getHost() + " port=" + url->getPort() + " dbname=" + url->getPath();
        pgurl += " user=" + url->getUser() + " password=" + url->getPassword();
        vector<string> options;
        if (url->hasParameter("statementTimeout")) {
            options.push_back("statementTimeout");
        }
        if (url->hasParameter("search_path")) {
            options.push_back("search_path");
        }
        /*
        if (!options.empty()) {
            pgurl += " options='";
            for (size_t idx = 0; idx < options.size(); idx++) {

            }
            pgurl += "'";
        }
        */
        if (url->hasParameter("statementTimeout") || url->hasParameter("search_path")) {
            pgurl += " options='";
            if (url->hasParameter("statementTimeout")) {
                pgurl += "--statement-timeout=" + url->getParameter("statementTimeout");
            }
            if (url->hasParameter("search_path")) {
                pgurl += " --search-path=" + url->getParameter("search_path");
            }
            pgurl += "'";
            // --pgurl += " options='--statement-timeout=" + url->getParameter("statementTimeout")+"'";
        }
        return pgurl;
    }

    bool PGQueryExecution::isResourceAvailable() {
        LOG_INFO("check connection " + getUrl()->getUrl(false,true,false));
        string pgurl = toPostgresUrl(getUrl());
        LOG_INFO("check:\n"+pgurl)
        return PGConnection::ping(pgurl);
    }
}
