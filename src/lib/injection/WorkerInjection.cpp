/*
 * WorkerInjection.cpp
 *
 *  Created on: Aug 29, 2014
 *      Author: arnd
 */

#include "WorkerInjection.h"

#include "utils/logging.h"
#include "type/oids.h"
#include "type/TypeRegistry.h"
#include "utils/RegExp.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("WorkerInjection");

WorkerInjection::~WorkerInjection() {
    LOG_INFO("use_count delete injector");
    for (auto& step:steps) {
        step.release();
    }
}

string WorkerInjection::inject(string query, map<string,shared_ptr<TableData>> dependencies, size_t copyThreshold) {
    LOG_DEBUG("called inject " << query);
    for (auto dep:dependencies) {
        if (dep.second == nullptr) {
            throw runtime_error("can't inject sql because of missing dependency '" + dep.first + "'");
        }
    }
    string sql = "DO $$DECLARE BEGIN ";
    string step = "";
    //step += "SET SESSION statement_timeout TO 1;\n select pg_sleep(2); \n";
    //sql += step;
    //steps.push_back(ExecutionStep{step,nullptr});
    // 1. Step
    for (pair<string,shared_ptr<TableData>> dependency:dependencies) {
        LOG_DEBUG("process dependency " << dependency.first);
        step += "CREATE TABLE IF NOT EXISTS " + dependency.first + " (\n";
        step += dependency.second->toColumnDefinitions();
        step += ");\n";
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
    // 2. step: get meta information
    RegExp re{R"(limit ([1-9]+[0-9]*))"};
    vector<RegExp::match> matches;
    step = query;
    bool foundLimit = false;
    do {
         matches = re.exec(step);
         if (!matches.empty()) {
             step = step.substr(0,matches[1].start) + "0" + step.substr(matches[1].end);
             foundLimit = true;
         }
    } while(!matches.empty());
    if (!foundLimit) {
        step += " limit 0";
    }
    step += ";\n";
    steps.push_back(ExecutionStep{Action::GET_COLUMN_TYPES,step,nullptr});
    sql += step;
    // 3. step: get binary result
    step = "COPY (" + query + ") TO STDOUT;\n";
    steps.push_back(ExecutionStep{Action::GET_RESULT,step,nullptr});
    sql += step;
    sql += " END; $$";
    LOG_DEBUG("query = " << sql);
    //this->dependencies = dependencies;
    return sql;
}

ExecutionStep& WorkerInjection::getStep(int stepNo) {
    return steps[stepNo];
}

}





