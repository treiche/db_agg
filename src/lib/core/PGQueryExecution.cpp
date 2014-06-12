/*
 * PGQueryExecution.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: arnd
 */

#include "PGQueryExecution.h"

#include <log4cplus/logger.h>

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

    /*
    PGQueryExecution::PGQueryExecution(string name, string id, shared_ptr<Url> url, string sql, vector<string> depName, DependencyInjector *dependencyInjector):
            QueryExecution(name,id,url,sql,depName,dependencyInjector) {
        queryExecutor.addEventListener(this);
    }
    */


    uint64_t PGQueryExecution::getRowCount(size_t stepNo) {
        ExecutionStep& step = getInjector()->getStep(stepNo);
        return step.getDependency()->getRowCount();
    }


    string PGQueryExecution::handleCopyIn(size_t stepNo, uint64_t startRow, uint64_t rows, uint64_t& rowsRead) {
        ExecutionStep& step = getInjector()->getStep(stepNo);
        uint32_t size;
        uint64_t rowCount = step.getDependency()->getRowCount();
        string data;
        rowsRead = 0;
        for (uint64_t row = startRow; (row < rowCount) && (rowsRead < rows); row++) {
            void *rawRow = step.getDependency()->getRawRow(row,size);
            data.append((char*)rawRow, size);
            rowsRead++;
        }
        return data;
    }

    void PGQueryExecution::handleCopyOut(size_t stepNo,string _data) {
         getData()->appendRaw((void*)_data.c_str(),_data.size());
    }

    void PGQueryExecution::handleTuples(size_t step, vector<pair<string, uint32_t> >& columns) {
        vector<pair<string, uint32_t>> cleaned;
        for (size_t idx = 0; idx < columns.size(); idx++) {
            auto column = columns[idx];
            if (column.first == "?column?") {
                LOG4CPLUS_DEBUG(LOG,"column " << idx << "has unknown name/type. assume type TEXT");
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
        // queryExecutor->addQuery(result->getId(), result->getConnectionUrl().getUrl(true,false,true), sql, eh);
        string injectedSql = this->inject(getSql(), 0);
        queryExecutor.addQuery(getId(),toPostgresUrl(getUrl()), injectedSql, this);
        setScheduled();
    }

    bool PGQueryExecution::process() {
        LOG4CPLUS_DEBUG(LOG,"process postgres query '" << getName() << "'");
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
        string pgurl = "host=" + url->getHost() + " port=" + to_string(url->getPort()) + " dbname=" + url->getPath();
        pgurl += " user=" + url->getUser() + " password=" + url->getPassword();
        if (url->hasParameter("statementTimeout")) {
            pgurl += " options='--statement-timeout=" + url->getParameter("statementTimeout")+"'";
        }
        return pgurl;
    }

    bool PGQueryExecution::isResourceAvailable() {
        LOG4CPLUS_INFO(LOG, "check connection " + getUrl()->getUrl(false,true,false));
        string pgurl = toPostgresUrl(getUrl());
        return PGConnection::ping(pgurl);
    }
}
