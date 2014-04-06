/*
 * SqlInjector.h
 *
 *  Created on: Mar 28, 2014
 *      Author: arnd
 */

#ifndef SQLINJECTOR_H_
#define SQLINJECTOR_H_

#include <string>
#include <map>
#include "table/TableData.h"

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
    TableData *dependency = nullptr;
public:
    ExecutionStep(Action action, std::string query, TableData *dependency):
        action(action),
        query(query),
        dependency(dependency) {}
    std::string getQuery() {
        return query;
    }
    TableData *getDependency() {
        return dependency;
    }
};



class DependencyInjector {
public:
    virtual std::string inject(std::string query, std::map<std::string,TableData*> dependencies, size_t copyThreshold) = 0;
    virtual ExecutionStep& getStep(int stepNo) = 0;
    virtual ~DependencyInjector() {}
};



}



#endif /* SQLINJECTOR_H_ */
