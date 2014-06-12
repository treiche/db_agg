/*
 * ResourceQueryExecution.h
 *
 *  Created on: Jun 11, 2014
 *      Author: arnd
 */

#ifndef RESOURCEQUERYEXECUTION_H_
#define RESOURCEQUERYEXECUTION_H_


#include "QueryExecution.h"

namespace db_agg {
class ResourceQueryExecution: public QueryExecution {
public:
    // ResourceQueryExecution(std::string name, std::string id, std::shared_ptr<Url> url, std::string sql, std::vector<std::string> depName, DependencyInjector *dependencyInjector);
    virtual void stop() override;
    virtual void schedule() override;
    virtual bool process() override;
    virtual void cleanUp() override;
    virtual bool isResourceAvailable() override;

};
}



#endif /* RESOURCEQUERYEXECUTION_H_ */
