/*
 * extension.h
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */

#ifndef EXTENSION_H_
#define EXTENSION_H_

#include <memory>
#include <string>
#include <map>

#include "sharding/ShardingStrategy.h"
#include "core/QueryExecution.h"

namespace db_agg {

enum class ComponentType {
    SHARDING_STRATEGY,
    QUERY_EXECUTION,
    DEPENDENCY_INJECTOR
};

class Extension {
public:
    virtual std::shared_ptr<ShardingStrategy> getShardingStrategy(std::string name);
    virtual QueryExecution *getQueryExecution(std::string name);
    virtual std::shared_ptr<DependencyInjector> getDependencyInjector(std::string name);
    virtual std::map<ComponentType,std::vector<std::string>> getProvidedComponents() = 0;
    virtual ~Extension() {};
};

}


#endif /* EXTENSION_H_ */
