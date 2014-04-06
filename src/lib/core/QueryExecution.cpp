/*
 * QueryExecution.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */

#include "QueryExecution.h"

#include <log4cplus/logger.h>

#include "type/oids.h"
#include "type/TypeRegistry.h"
#include "table/CsvTableData.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("QueryExecution"));

    QueryExecution::QueryExecution() {}

    QueryExecution::QueryExecution(std::string name, std::string id, std::string connectionUrl, std::string sql, std::vector<std::string> depNames, DependencyInjector *dependencyInjector):
            id(id),
            connectionUrl(connectionUrl),
            sql(sql),
            name(name),
            dependencyInjector(dependencyInjector) {
        for (auto& depName:depNames) {
            dependencies[depName] = nullptr;
        }
    }

    QueryExecution::~QueryExecution() {
        LOG4CPLUS_TRACE(LOG, "delete query execution");
        if (data!=nullptr) {
            delete data;
        }
    }

    void QueryExecution::addTransition(Transition *t) {
        transitions.push_back(t);
    }

    void QueryExecution::addIncomingTransition(Transition *t) {
        incomingTransitions.push_back(t);
    }

    void QueryExecution::addDependency(string name, TableData *data) {
        if (dependencies.find(name)==dependencies.end()) {
            throw runtime_error("no dependency '" + name + "' declared");
        }
        dependencies[name] = data;
    }

    string QueryExecution::inject(string query, size_t copyThreshold) {
        assert(dependencyInjector != nullptr);
        LOG4CPLUS_DEBUG(LOG, "called inject " << query << " di = " << dependencyInjector);
        return dependencyInjector->inject(query,dependencies,copyThreshold);
    }

    uint64_t QueryExecution::getRowCount(size_t stepNo) {
        ExecutionStep& step = dependencyInjector->getStep(stepNo);
        return step.getDependency()->getRowCount();
    }


    string QueryExecution::handleCopyIn(size_t stepNo, uint32_t row) {
        ExecutionStep& step = dependencyInjector->getStep(stepNo);
        uint32_t size;
        void *rawRow = step.getDependency()->getRawRow(row,size);
        return string((const char*)rawRow,size);
    }

    void QueryExecution::handleCopyOut(size_t stepNo,string _data) {
         data->appendRaw((void*)_data.c_str(),_data.size());
    }

    void QueryExecution::handleTuples(size_t step, vector<pair<string, uint32_t> >& columns) {
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
        data = new CsvTableData(cleaned);
    }

    TableData * QueryExecution::getResult() {
        return data;
    }

    void QueryExecution::setResult(TableData *data) {
        LOG4CPLUS_DEBUG(LOG,"set result to " << data);
        this->data = data;
    }

    bool QueryExecution::isComplete() {
        bool complete = true;
        for (auto& dep:dependencies) {
            complete &= (dep.second!=nullptr);
        }
        return complete;
    }

    void QueryExecution::doTransitions() {
        if (!transitions.empty()) {
            LOG4CPLUS_DEBUG(LOG, "do transitions for query " << sql);
        }
        for (Transition *t:transitions) {
            t->doTransition();
        }
    }

    std::ostream& operator<<(std::ostream& cout,const QueryExecution& qe) {
        cout << "QueryExecution[sql=" << qe.sql << "]";
        return cout;
    }

}

