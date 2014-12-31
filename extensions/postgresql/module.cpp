/*
 * testmodel.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */
#include <iostream>
#include <memory>
#include "db_agg.h"
#include "PGQueryExecution.h"
#include "PostgresqlExecution.h"

#include <log4cplus/logger.h>


using namespace std;
using namespace log4cplus;
using namespace db_agg;

DECLARE_LOGGER("PostgresqlExtension");

class PostgresqlExtension : public Extension {
private:
    map<ComponentType,vector<string>> components{
        {ComponentType::QUERY_EXECUTION, { "postgresql"}}
    };
public:
    virtual ~PostgresqlExtension() override;
    virtual shared_ptr<ShardingStrategy> getShardingStrategy(string name) override;
    virtual QueryExecution *getQueryExecution(std::string name) override;
    virtual map<ComponentType,vector<string>> getProvidedComponents() override;
};

map<ComponentType,vector<string>> PostgresqlExtension::getProvidedComponents() {
    return components;
}

QueryExecution *PostgresqlExtension::getQueryExecution(std::string name) {
    if (name == "postgresql") {
        // return new db_agg::PGQueryExecution();
        return new db_agg::PostgresqlExecution();
    }
    return nullptr;
}

shared_ptr<db_agg::ShardingStrategy> PostgresqlExtension::getShardingStrategy(string name) {
    LOG_DEBUG("called getShardingStrategy(" << name << ")" );
    return nullptr;
}

PostgresqlExtension::~PostgresqlExtension() {
    LOG_DEBUG("called destructor");
}

extern "C" {
db_agg::Extension *getExtension() {
    return new PostgresqlExtension();
}
}
