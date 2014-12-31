/*
 * NoResultInjector.cpp
 *
 *  Created on: Mar 29, 2014
 *      Author: arnd
 */

#include "NoResultInjector.h"

#include "utils/logging.h"
#include "type/oids.h"
#include "type/TypeRegistry.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("NoResultInjector");

NoResultInjector::~NoResultInjector() {

}

string NoResultInjector::inject(string query, deque<Port*> dependencies, size_t copyThreshold) {
    LOG_DEBUG("called inject " << query);
    for (auto dep:dependencies) {
        if (dep->getResult().get() == nullptr) {
            throw runtime_error("can't inject sql because of missing dependency '" + dep->getName() + "'");
        }
    }
    string sql;
    string step = "";
    //step += "SET SESSION statement_timeout TO 1;\n select pg_sleep(2); \n";
    //sql += step;
    //steps.push_back(ExecutionStep{step,nullptr});
    // 1. Step
    for (auto dependency:dependencies) {
        LOG_DEBUG("process dependency " << dependency->getName());
        step = "CREATE TEMPORARY TABLE " + dependency->getName() + "(\n";
        step += dependency->getResult()->toColumnDefinitions();
        step += ") ON COMMIT DROP;\n";
        sql += step;
        steps.push_back(ExecutionStep{Action::COMMAND,step,nullptr});
        if (dependency->getResult()->getRowCount() > 0) {
            if (dependency->getResult()->getRowCount() < copyThreshold) {
                step = "INSERT INTO " + dependency->getName() + " VALUES " + dependency->getResult()->toSqlValues() + ";\n";
                steps.push_back(ExecutionStep{Action::PUSH_DEPENDENCY, step,dependency->getResult()});
            } else {
                step = "COPY " + dependency->getName() + " FROM STDIN;\n";
                steps.push_back(ExecutionStep{Action::PUSH_DEPENDENCY,step,dependency->getResult()});
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





