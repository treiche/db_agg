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
#include <memory>
#include "table/TableData.h"
#include "ExecutionStep.h"

namespace db_agg {

class DependencyInjector {
public:
    virtual std::string inject(std::string query, std::map<std::string,std::shared_ptr<TableData>> dependencies, size_t copyThreshold) = 0;
    virtual ExecutionStep& getStep(int stepNo) = 0;
    virtual ~DependencyInjector() {}
};
}

#endif /* SQLINJECTOR_H_ */
