/*
 * ExecutionStep.cpp
 *
 *  Created on: Aug 23, 2014
 *      Author: arnd
 */

#include "ExecutionStep.h"
#include "utils/logging.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("ExecutionStep")

string ExecutionStep::getQuery() {
    return query;
}
shared_ptr<TableData> ExecutionStep::getDependency() {
    return dependency;
}
void ExecutionStep::release() {
    if (dependency) {
        LOG_INFO("use_count " << dependency.use_count());
        dependency.reset();
    }
}


}


