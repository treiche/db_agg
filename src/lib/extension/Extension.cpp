/*
 * Extension.cpp
 *
 *  Created on: Jun 12, 2014
 *      Author: arnd
 */

#include "extension/Extension.h"

using namespace std;

namespace db_agg {

shared_ptr<ShardingStrategy> Extension::getShardingStrategy(std::string name) {
    return nullptr;
}

QueryExecution *Extension::getQueryExecution(string name) {
    return nullptr;
}

shared_ptr<DependencyInjector> Extension::getDependencyInjector(string name) {
    return nullptr;
}

}

