/*
 * testmodel.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */
#include <iostream>
#include <memory>
#include "db_agg.h"
#include "SoapExecution.h"

#include <log4cplus/logger.h>


using namespace std;
using namespace log4cplus;
using namespace db_agg;

DECLARE_LOGGER("SoapExtension");

class SoapExtension : public Extension {
private:
    map<ComponentType,vector<string>> components{
        {ComponentType::QUERY_EXECUTION, { "soap"}}
    };
public:
    virtual ~SoapExtension() override;
    virtual shared_ptr<ShardingStrategy> getShardingStrategy(string name) override;
    virtual QueryExecution *getQueryExecution(std::string name) override;
    virtual map<ComponentType,vector<string>> getProvidedComponents() override;
};

map<ComponentType,vector<string>> SoapExtension::getProvidedComponents() {
    return components;
}

QueryExecution *SoapExtension::getQueryExecution(std::string name) {
    if (name == "soap") {
        return new db_agg::SoapExecution();
    }
    return nullptr;
}

shared_ptr<db_agg::ShardingStrategy> SoapExtension::getShardingStrategy(string name) {
    LOG4CPLUS_DEBUG(LOG, "called getShardingStrategy(" << name << ")" );
    return nullptr;
}

SoapExtension::~SoapExtension() {
    LOG_DEBUG("called destructor");
}

extern "C" {
db_agg::Extension *getExtension() {
    return new SoapExtension();
}
}
