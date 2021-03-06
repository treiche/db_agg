/*
 * Builtin.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#include "Builtin.h"
#include "sharding/MD5ShardingStrategy.h"
#include "sharding/ModuloShardingStrategy.h"
#include "sharding/ExplicitShardingStrategy.h"
#include "core/ResourceQueryExecution.h"
#include "injection/DefaultDependencyInjector.h"
#include "injection/NoResultInjector.h"
#include "injection/WorkerInjection.h"

#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
DECLARE_LOGGER("Builtin");

map<ComponentType,vector<string>> Builtin::components = {
        {ComponentType::SHARDING_STRATEGY,{"md5","explicit","modulo"}},
        {ComponentType::QUERY_EXECUTION, {"resource"}},
        {ComponentType::DEPENDENCY_INJECTOR, {"default","noresult","worker"}}
};

shared_ptr<ShardingStrategy> Builtin::getShardingStrategy(std::string name) {
    if (name == "md5") {
        return shared_ptr<ShardingStrategy>(new MD5ShardingStrategy());
    } else if (name == "modulo") {
        return shared_ptr<ShardingStrategy>(new ModuloShardingStrategy());
    } else if (name == "explicit") {
        return shared_ptr<ShardingStrategy>(new ExplicitShardingStrategy());
    }
    return nullptr;
}

QueryExecution *Builtin::getQueryExecution(std::string name) {
    if (name == "resource") {
        return new ResourceQueryExecution();
    }
    return nullptr;
}

shared_ptr<DependencyInjector> Builtin::getDependencyInjector(std::string name) {
    if (name == "default") {
        return shared_ptr<DependencyInjector>(new DefaultDependencyInjector());
    } else if (name == "noresult") {
        return shared_ptr<DependencyInjector>(new NoResultInjector());
    } else if (name == "worker") {
        return shared_ptr<DependencyInjector>(new WorkerInjection());
    }
    return nullptr;
}

map<ComponentType,vector<string>> Builtin::getProvidedComponents() {
    return components;
}

Builtin::~Builtin() {};


}



