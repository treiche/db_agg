/*
 * Builtin.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#include "Builtin.h"
#include "sharding/MD5ShardingStrategy.h"
#include "core/PGQueryExecution.h"
#include "core/ResourceQueryExecution.h"

#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Builtin"));

map<ComponentType,vector<string>> Builtin::components = {
        {ComponentType::SHARDING_STRATEGY,{"md5"}},
        {ComponentType::QUERY_EXECUTION, {"resource","postgres"}},
        {ComponentType::DEPENDENCY_INJECTOR, {"default","noresult"}}
};

shared_ptr<ShardingStrategy> Builtin::getShardingStrategy(std::string name) {
    if (name == "md5") {
        return shared_ptr<ShardingStrategy>(new MD5ShardingStrategy());
    }
    return nullptr;
}

QueryExecution *Builtin::getQueryExecution(std::string name) {
    if (name == "postgres") {
        return new PGQueryExecution();
    } else if (name == "resource") {
        return new ResourceQueryExecution();
    }
    return nullptr;
}

shared_ptr<DependencyInjector> Builtin::getDependencyInjector(std::string name) {
    if (name == "default") {
        return shared_ptr<DependencyInjector>(new DefaultDependencyInjector());
    } else if (name == "noresult") {
        return shared_ptr<DependencyInjector>(new NoResultInjector());
    }
    return nullptr;
}

map<ComponentType,vector<string>> Builtin::getProvidedComponents() {
    return components;
}

Builtin::~Builtin() {};


}



