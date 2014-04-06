/*
 * NoResultInjector.h
 *
 *  Created on: Mar 29, 2014
 *      Author: arnd
 */

#ifndef NORESULTINJECTOR_H_
#define NORESULTINJECTOR_H_


#include <deque>
#include <map>
#include <string>
#include "DependencyInjector.h"
#include "table/TableData.h"

namespace db_agg {

class NoResultInjector : public DependencyInjector {
private:
    std::deque<ExecutionStep> steps;
public:
    virtual std::string inject(std::string query, std::map<std::string,TableData*> dependencies, size_t copyThreshold) override;
    virtual ExecutionStep& getStep(int stepNo) override;
    virtual ~NoResultInjector() override;
};

}




#endif /* NORESULTINJECTOR_H_ */
