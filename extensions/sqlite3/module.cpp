/*
 * testmodel.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: arnd
 */
#include <iostream>
#include <memory>
#include "db_agg.h"
#include "Sqlite3Execution.h"

#include <log4cplus/logger.h>


using namespace std;
using namespace log4cplus;
using namespace db_agg;

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Sqlite3Extension"));

class Sqlite3Extension : public Extension {
private:
    map<ComponentType,vector<string>> components{
        {ComponentType::QUERY_EXECUTION, { "sqlite3"}}
    };
public:
    virtual ~Sqlite3Extension() override;
    virtual shared_ptr<ShardingStrategy> getShardingStrategy(string name) override;
    virtual QueryExecution *getQueryExecution(std::string name) override;
    virtual map<ComponentType,vector<string>> getProvidedComponents() override;
};

map<ComponentType,vector<string>> Sqlite3Extension::getProvidedComponents() {
    return components;
}

QueryExecution *Sqlite3Extension::getQueryExecution(std::string name) {
    if (name == "sqlite3") {
        return new db_agg::Sqlite3Execution();
    }
    return nullptr;
}

shared_ptr<db_agg::ShardingStrategy> Sqlite3Extension::getShardingStrategy(string name) {
    LOG4CPLUS_DEBUG(LOG, "called getShardingStrategy(" << name << ")" );
    return nullptr;
}

Sqlite3Extension::~Sqlite3Extension() {
    LOG4CPLUS_DEBUG(LOG, "called destructor");
}

extern "C" {
db_agg::Extension *getExtension() {
    return new Sqlite3Extension();
}
}
