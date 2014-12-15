/*
 * testmodel.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */
#include <iostream>
#include <memory>
#include "db_agg.h"
#include "MemcachedQuery.h"

#include <log4cplus/logger.h>


using namespace std;
using namespace log4cplus;
using namespace db_agg;

DECLARE_LOGGER("MemcachedExtension");

class MemcachedExtension : public Extension {
private:
    map<ComponentType,vector<string>> components{
        {ComponentType::QUERY_EXECUTION, { "memcached2"}}
    };
public:
    virtual ~MemcachedExtension() override;
    virtual shared_ptr<ShardingStrategy> getShardingStrategy(string name) override;
    virtual QueryExecution *getQueryExecution(std::string name) override;
    virtual map<ComponentType,vector<string>> getProvidedComponents() override;
};

map<ComponentType,vector<string>> MemcachedExtension::getProvidedComponents() {
    return components;
}

QueryExecution *MemcachedExtension::getQueryExecution(std::string name) {
    if (name == "memcached2") {
        return new memcached::MemcachedQuery();
    }
    return nullptr;
}

shared_ptr<db_agg::ShardingStrategy> MemcachedExtension::getShardingStrategy(string name) {
    LOG_DEBUG("called getShardingStrategy(" << name << ")" );
    return nullptr;
}

MemcachedExtension::~MemcachedExtension() {
    LOG_DEBUG("called destructor");
}

extern "C" {
db_agg::Extension *getExtension() {
    return new MemcachedExtension();
}
}
