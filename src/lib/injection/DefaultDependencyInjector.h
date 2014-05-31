/*
 * DefaultDependencyInjector.h
 *
 *  Created on: Mar 29, 2014
 *      Author: arnd
 */

#ifndef DEFAULTDEPENDENCYINJECTOR_H_
#define DEFAULTDEPENDENCYINJECTOR_H_

#include <deque>
#include <map>
#include <string>
#include "DependencyInjector.h"
#include "table/TableData.h"

namespace db_agg {

class DefaultDependencyInjector : public DependencyInjector {
private:
    std::deque<ExecutionStep> steps;
public:
    virtual std::string inject(std::string query, std::map<std::string,std::shared_ptr<TableData>> dependencies, size_t copyThreshold) override;
    virtual ExecutionStep& getStep(int stepNo) override;
    virtual ~DefaultDependencyInjector() override;
};

}


#endif /* DEFAULTDEPENDENCYINJECTOR_H_ */
