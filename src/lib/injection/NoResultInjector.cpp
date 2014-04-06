/*
 * NoResultInjector.cpp
 *
 *  Created on: Mar 29, 2014
 *      Author: arnd
 */

#include "NoResultInjector.h"

#include <log4cplus/logger.h>
#include "type/oids.h"
#include "type/TypeRegistry.h"
#include "table/CsvTableData.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("NoResultInjector"));

NoResultInjector::~NoResultInjector() {

}

string NoResultInjector::inject(string query, map<string,TableData*> dependencies, size_t copyThreshold) {
    LOG4CPLUS_DEBUG(LOG, "called inject " << query);
    for (auto dep:dependencies) {
        if (dep.second == nullptr) {
            throw runtime_error("can't inject sql because of missing dependency '" + dep.first + "'");
        }
    }
    string sql;
    string step = "";
    //step += "SET SESSION statement_timeout TO 1;\n select pg_sleep(2); \n";
    //sql += step;
    //steps.push_back(ExecutionStep{step,nullptr});
    // 1. Step
    for (pair<string,TableData*> dependency:dependencies) {
        LOG4CPLUS_DEBUG(LOG, "process dependency " << dependency.first);
        step = "CREATE TEMPORARY TABLE " + dependency.first + "(\n";
        step += dependency.second->toColumnDefinitions();
        step += ") ON COMMIT DROP;\n";
        sql += step;
        steps.push_back(ExecutionStep{Action::COMMAND,step,nullptr});
        if (dependency.second->getRowCount() > 0) {
            if (dependency.second->getRowCount() < copyThreshold) {
                step = "INSERT INTO " + dependency.first + " VALUES " + dependency.second->toSqlValues() + ";\n";
                steps.push_back(ExecutionStep{Action::PUSH_DEPENDENCY, step,dependency.second});
            } else {
                step = "COPY " + dependency.first + " FROM STDIN;\n";
                steps.push_back(ExecutionStep{Action::PUSH_DEPENDENCY,step,dependency.second});
            }
            sql += step;
        }
    }
    sql += query + ";\n";
    steps.push_back(ExecutionStep{Action::COMMAND,step,nullptr});
    step = "select 1;\n";
    sql += step;
    steps.push_back(ExecutionStep{Action::GET_COLUMN_TYPES,step,nullptr});
    step = "COPY ( select 1 ) TO STDOUT;\n";
    sql += step;
    steps.push_back(ExecutionStep{Action::GET_RESULT,step,nullptr});
    return sql;
}

ExecutionStep& NoResultInjector::getStep(int stepNo) {
    return steps[stepNo];
}

}





