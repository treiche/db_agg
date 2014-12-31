/*
 * WorkerInjection.h
 *
 *  Created on: Aug 29, 2014
 *      Author: arnd
 */

#ifndef WORKERINJECTION_H_
#define WORKERINJECTION_H_

#include <deque>
#include <map>
#include <string>
#include "DependencyInjector.h"
#include "table/TableData.h"

namespace db_agg {

class WorkerInjection : public DependencyInjector {
private:
    std::deque<ExecutionStep> steps;
public:
    virtual std::string inject(std::string query, std::deque<Port*> dependencies, size_t copyThreshold) override;
    virtual ExecutionStep& getStep(int stepNo) override;
    virtual ~WorkerInjection() override;
};

}




#endif /* WORKERINJECTION_H_ */
