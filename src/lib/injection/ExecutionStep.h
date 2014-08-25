/*
 * ExecutionStep.h
 *
 *  Created on: Aug 23, 2014
 *      Author: arnd
 */

#ifndef EXECUTIONSTEP_H_
#define EXECUTIONSTEP_H_

#include "table/TableData.h"
#include <memory>

namespace db_agg {

enum class Action {
    COMMAND,
    PUSH_DEPENDENCY,
    GET_RESULT,
    GET_COLUMN_TYPES
};

class ExecutionStep {
private:
    Action action;
    std::string query;
    std::shared_ptr<TableData> dependency;
public:
    ExecutionStep(Action action, std::string query, std::shared_ptr<TableData> dependency):
        action(action),
        query(query),
        dependency(dependency) {}
    std::string getQuery();
    std::shared_ptr<TableData> getDependency();
    void release();
};

}



#endif /* EXECUTIONSTEP_H_ */
